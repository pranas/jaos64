%include "config.asm"

bits 16
org stage2

; clear segments
	cli
	xor ax, ax
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov sp, stack-22
	sti

; print message
	mov si, preparing
	call Print

;
; While we're still in Real mode we should collect
; some information for our kernel
; 

; Get Memory Map
	mov ax, 0x600
	mov es, ax
	mov di, 0
	call BiosGetMemoryMap
	; BP = count of entries in map

    mov si, loading
    call Print
	
;
; Now we can bootstrap to long mode
; for our kernel
;
	
; Enable A20
	;pusha
	mov ax, 0x2401
	int 0x15
	;popa

    cli
	
; Load global descriptor table

    lgdt [GDT32.pointer]

; Try to enter protected mode
	mov eax, cr0
	or  eax, 1
	mov cr0, eax
	
; Jump setting code descriptor (GDT32.code)
	jmp GDT32.code:protectedMode
	
; Global descriptor table

    GDT32: 
    .null equ $ - GDT32
    dq 0x0000000000000000 ; null descriptor
    .code equ $ - GDT32
    dq 0x00cf9a000000ffff ; cs
    .data equ $ - GDT32
    dq 0x00cf92000000ffff ; ds
    .pointer:
	dw $ - GDT32 - 1            ; size of GDT minus 1
	dd GDT32                    ; base of GDT	

	%include "print.asm"
	%include "memory.asm"

bits 32
protectedMode:

; set other segments to data descriptors
	mov ax, 0x10
	mov ds, ax
	mov ss, ax
	mov es, ax
	mov esp, 0x90000 ; why 0x90000 ? I dont know :D
	
; Prepare 64-bit stuff

; We will put our paging tables from 0x1000 to 0x5000
; First let's clear those tables

; PML4T - 0x1000
; PDPT - 0x2000
; PDT - 0x3000
; PT - 0x4000

	mov di, 0x1000
	xor ax, ax
	mov cx, 0x4000
	rep stosb

; Lets make PML4T[0] point to the PDPT and so on

	mov di, 0x1000
	mov WORD [di], 0x2003
	mov di, 0x2000
	mov WORD [di], 0x3003
	mov di, 0x3000
	mov WORD [di], 0x4003

; 3 is used to set first two bits
; (it's Present and RW flags)

; This will map first 2MB to first 2MB on physical memory

	mov di, 0x4000				; Our PT starts there
	mov ebx, 0x00000003			; 3 again to set first two bits
	mov cx, 512					; Loop

	.setPageEntry:
	mov DWORD [di], ebx
	add ebx, 0x1000
	add di, 8					; Move to next page entry
	loop .setPageEntry

; Now we should enable PAE-paging by setting the PAE-bit in the CR4

	mov eax, cr4
	or eax, 00000000000000000000000000100000b	; PAE bit is 6th bit
	mov cr4, eax

; CR3 should point to PML4T

	mov eax, 0x00001000			; Remember, our PML4T starts at 0x1000
	mov cr3, eax

; Now paging is set up, but it isn't enabled yet
; We should set the long mode bit in the EFER MSR

	mov ecx, 0xC0000080			; Set the C-register to 0xC0000080, which is the EFER MSR
	rdmsr						; Read from the model-specific register
	or eax, 00000000000000000000000100000000b ; LM-bit is 9th bit
	wrmsr						; Write to the model-specific register

; Enabling paging

	mov eax, cr0
	or eax, 10000000000000000000000000000000b ; PG-bit is 32nd bit
	mov cr0, eax

; Now we're in compatibility mode.

	lgdt [GDT64.pointer]		; Load the 64-bit global descriptor table
	jmp GDT64.code:longMode		; Set the code segment and enter 64-bit long mode

; Global Descriptor Table (64-bit).
	GDT64:
    .null equ $ - GDT64
    dq 0x0000000000000000
    .code equ $ - GDT64
    dq 0x0020980000000000   
    .data equ $ - GDT64
    dq 0x0000900000000000   
	.pointer:
	dw $ - GDT64 - 1 			; 16-bit Size (Limit)
	dq GDT64 					; 64-bit Base Address

bits 64

%include "loadelf.asm"

;%define	boot_drive 			bp-2
%define	bytes_per_cluster 	0x9000-6
%define	fat_begin_lba		0x9000-10
%define	cluster_begin_lba	0x9000-14
%define current_cluster		0x9000-18

longMode: 
    ;cli
; Set segment registers to the data descriptor.
	mov ax, GDT64.data			
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov rsp, 0x90000         ; stack starts at 36kb
	xchg bx, bx
;
; Load kernel file using in/out
;
    xor rax, rax
    mov eax, [FAT32.root_cluster]
    mov [current_cluster], eax
    call cluster2lba
    ; rax lba
    ; dl  sector count
    ; rdi destination
    mov dl, [FAT32.sectors_per_cluster]
    mov rdi, 0x10000
    call read
    
    xchg bx, bx
    
; Root dir from 0x10000
; lets travel through root directory and look for our file to load
searchFile:
	mov rax, 0x10000-32
	movzx rbx, WORD [bytes_per_cluster]
	add rbx, 0x10000

nextEntry:
	add rax, 32
	cmp rax, rbx		; bx = end of cluster in memory
	;je notFound
	jne testingJump
	jmp notFound
	testingJump:

	mov rdi, fileName
	mov rsi, rax
	mov rcx, 11
	; compare names
	repe cmpsb
	; if not the same, try next entry
	jnz nextEntry

	; file found

	; File entry looks like this:
	;
	; Filename 		times 11 db
	; Attribute 	db
	; ____ 			dq
	; Cluster High 	dw
	; ____ 			dd
	; Cluster Low 	dw
	; Size 			dd
    ; si is already +11 in entry
    movzx rax, WORD [rsi+9]
    mov rbx, 0x100
    mul rbx
    mul rbx
    mov ax, [rsi+15]
    
    mov [current_cluster], eax
    call cluster2lba
    mov dl, [FAT32.sectors_per_cluster]
    mov rdi, 0x100000
    call read
    
    xchg bx, bx

; load more if there is

oneMoreCluster:   
    ;add di, [bytes_per_cluster] 
    mov eax, [current_cluster]
    call getNextCluster
    jc loaded           ; if eof
    mov [current_cluster], eax
    call cluster2lba
    mov dl, [FAT32.sectors_per_cluster]
    call read
    jmp oneMoreCluster

loaded:
    ; we loaded our file completely
    xchg bx, bx
    jmp parse_elf

notFound:
    ; call a procedure to get next cluster
    mov eax, [current_cluster]
    call getNextCluster
    ; if returned something
    jnc searchFile

    ; File not found!

    xchg bx, bx
    
;buffer times 512 db 1
;
; Parse ELF file
;
parse_elf:
	mov rbx, 0x100000 ;qword kernel   ; 0x5000 + stage2 offset, start of kernel ELF
	call loadelf

	mov r12, rbx
	xchg bx, bx             ; debugger trap
	mov rdi, 0x0004BEEF     ; integer/pointer for first arg of kernel
	call r12                ; call kernel

	hlt


fileName			db "KERNEL     "
loading 			db "Loading kernel...", 13, 10, 0
preparing 			db "Preparing to load kernel...", 13, 10, 0

;
; Procedure to load next cluster
;   IN:
;       rax - current cluster
;
;	OUT:
;       rax - next cluster
;
;		Carry if EOF

getNextCluster:
    push rdx
    push rbx
    push rdi
    
	; check if we're already at EOF
	and eax, FAT_Cluster_Mask	; cluster mask
	cmp eax, FAT_EOF
	jge .eof 					; greater or equal
	
	; do the math to find out wich FAT sector to read
	xor edx, edx 				; div 32bit reg divides edx:eax
	xor ebx, ebx
	mov bx, [FAT32.bytes_per_sector]
	shr bx, 2 					; bx = bx / 4 (each entry is 32bits (4 B) long)
	div ebx
	
	push rdx  					; div remainder
		
	; now read that sector
	add eax, [fat_begin_lba]
	mov dl, 1
    mov rdi, 0x10000
    call read

	; now let's do the math again to find out wich entry of FAT do we need
	; (use remainder)
	
	pop rdx						; remainder
	sal edx, 2					; edx = edx * 4
	
	mov edi, 0x10000 ; buffer
	add edi, edx
	
	mov eax, [edi]			; next cluster

	and eax, FAT_Cluster_Mask
	cmp eax, FAT_EOF
	jge .eof 					; greater or equal
	
    pop rdi
    pop rbx
    pop rdx	
	clc
	ret
.eof:
    pop rdi
    pop rbx
    pop rdx
	stc	
	ret

;
; Procedure to read from disk
;   IN:
;       rax - LBA
;       rdi - destination
;       dl  - sector count
;       
;   OUT:
;       CF if error
;
read:
    push rcx
    

    mov ecx, eax
    
    mov al, dl          ; move sector count to al
    
    mov dx,1f2h         ;Sector count port
    ;mov al,1            ;Read one sector
    out dx,al
    
    mov al, cl		; ecx currently holds LBA
    inc edx			; port 1f3 -- LBAlow
    out dx, al

    mov al, ch
    inc edx			; port 1f4 -- LBAmid
    out dx, al

    bswap ecx
    mov al, ch		; bits 16 to 23 of LBA
    inc edx			; port 1f5 -- LBAhigh
    out dx, al

    mov al, cl            ; bits 24 to 28 of LBA
    or al, 0xe0 ;byte [esi + dd_sbits]    ; master/slave flag | 0xe0
    inc edx               ; port 1f6 -- drive select
    out dx, al

    inc edx			; port 1f7 -- command/status
    mov al, 0x20		; send "read" command to drive
    out dx, al
    
;7	BSY		Drive is preparing to accept/send data -- wait until this bit clears. If it never
;			clears, do a Software Reset. Technically, when BSY is set, the other bits in the
;			Status byte are meaningless.

; when BSY is clear, check

;3	DRQ		Set when the drive has PIO data to transfer, or is ready to accept PIO data.


still_going:
   in al,dx
   test al,8            ;This means the sector buffer requires servicing.
   jz still_going     ;Don't continue until the sector buffer is ready.
   mov rcx,512/2        ;One sector /2
   ;mov rdi, 0x10000 
   mov dx,1f0h         ;Data port - data comes in and out of here.
   rep insw

   pop rcx
   ret
   
;
; Procedure to get LBA from cluster number
;	IN:
;		eax - cluster number
;	OUT:
;		eax - LBA
;
;	LBA = cluster_begin_lba + (cluster_number - 2) * sectors_per_cluster;
;

cluster2lba:
	push rcx
	
	xor ecx, ecx
	dec eax
	dec eax
	mov cl, BYTE [FAT32.sectors_per_cluster]
	mul ecx
	add eax, [cluster_begin_lba]
	
	pop rcx
	ret

absolute stage1+512

%include "fat32_volumeid.asm"