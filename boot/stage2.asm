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

;%define	boot_drive 			bp-2
%define	bytes_per_cluster 	0x9000-6
%define	fat_begin_lba		0x9000-10
%define	cluster_begin_lba	0x9000-14
%define current_cluster		0x9000-18

%include "diskio64.asm"
%include "loadelf.asm"

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
; Let's look for kernel file in disk
;
    xchg bx, bx
    mov rdi, fileName
    call searchFile
    jnc kernelFound
    
    ;
    ; Kernel not found!
    ;
    
    hlt
;
; Load kernel to memory
;
kernelFound:
    mov rdi, kernel
    call loadFile
        
;
; Parse ELF file
;
	mov rbx, kernel ;qword kernel   ; 0x5000 + stage2 offset, start of kernel ELF
	call loadelf

	mov r12, rbx
	xchg bx, bx             ; debugger trap
	mov rdi, 0x0004BEEF     ; integer/pointer for first arg of kernel
	call r12                ; call kernel

	hlt


fileName			db "KERNEL     "
loading 			db "Loading kernel...", 13, 10, 0
preparing 			db "Preparing to load kernel...", 13, 10, 0

absolute stage1+512

%include "fat32_volumeid.asm"