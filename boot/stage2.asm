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

; print loading message
	mov si, loading
	call Print

;
; Let's look for kernel file and load it
;

	%define	boot_drive 			stack-2
	%define	bytes_per_cluster 	stack-6
	%define	fat_begin_lba		stack-10
	%define	cluster_begin_lba	stack-14
	%define current_cluster		stack-18

	xor eax, eax
	mov al, [FAT32.sectors_per_cluster]
	mov [AddressPacket.sectors], ax
	
	mov eax, [FAT32.root_cluster]
	and eax, FAT_Cluster_Mask
	mov [current_cluster], eax
	call clusterLBA
	mov [AddressPacket.lba], eax
	mov WORD [AddressPacket.buffer], 0x0000
	mov si, AddressPacket
	call readDisk
	jc $
	
searchFile:
	xor bx, bx
	mov ax, 0x0000-32
	mov bx, [bytes_per_cluster]
	add ebx, 0
	mov dx, 0x1000
	mov ds, dx
nextEntry:
	add ax, 32
	cmp ax, bx		; bx = end of cluster in memory
	je notFound



	mov di, fileName
	mov si, ax
	mov cx, 11
	; compare names
	repe cmpsb
	; if not the same, try next entry
	jnz nextEntry

	; file found

	mov ax, [si+9]			; si is already +11 in entry
	push ax					; push low word
	mov ax, [si+15]			
	push ax					; push high word
	pop eax 				; eax = 1st cluster of file
	jmp Found

notFound:
	; call a procedure to get next cluster
	call getNextCluster
	; if returned something
	jnc searchFile

	; File not found!
	mov si, kernelNotFound
	call Print
	hlt
	
;
;
;

	mov [current_cluster], eax
	call clusterLBA

	mov WORD [AddressPacket.buffer], 0x0000
	mov [AddressPacket.lba], eax
	mov ax, 0x1000
	mov ds, ax
	mov si, AddressPacket
	call readDisk
	
; load more if there is

oneMoreCluster:	
	xor ebx, ebx
	xor eax, eax
	mov ax, [AddressPacket.buffer]
	mov bx, [bytes_per_cluster]
	add eax, ebx
	jno .good2go
	mov [AddressPacket.buffer+2], bx
	add bx, 0x1000
	mov [AddressPacket.buffer+2], bx
	.good2go:
	mov [AddressPacket.buffer], eax

	call getNextCluster
	jnc oneMoreCluster		; not EOF

	; we loaded our file completely
	
	jmp Loaded


;
; clusterLBA, readDisk
;

	%include "diskio.asm"

	AddressPacket:
	.size			db 16
					db 0 			; always zero
	.sectors		dw 0x0001
	.buffer			dw 0x0000
					dw 0x1000
	.lba			dd 0
	.lba48			dd 0 			; used by 48 bit LBAs

Loaded:

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
	
;
; Now we can bootstrap to long mode
; for our kernel
;
	
; Enable A20
	pusha
	mov ax, 0x2401
	int 0x15
	popa
	
; Setup global descriptor table

	call loadGDT
	
; Try to enter protected mode
	cli ; interrupts are deadly to us from now on
	mov eax, cr0
	or  eax, 1
	mov cr0, eax
	
; Jump to code descriptor because CS is wrong now
	jmp 0x08:Stage3 ; 0x8 is the code descriptor offset
	
	%include "print.asm"
	%include "gdt.asm"
	%include "memory.asm"

; welcome to the kingdom of 32 bits
bits 32

Stage3:

; set other segments to data descriptors
	mov ax, 0x10 ;0x0000506a
	mov ds, ax
	mov ss, ax
	mov es, ax
	mov esp, 0x90000 ; why 0x90000 ? I dont know :D
	
; Stage3 will prepare 64-bit stuff

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

	.SetPageEntry:
	mov DWORD [di], ebx
	add ebx, 0x1000
	add di, 8					; Move to next page entry
	loop .SetPageEntry

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

	lgdt [GDT64.Pointer]		; Load the 64-bit global descriptor table
	jmp GDT64.Code:Stage4		; Set the code segment and enter 64-bit long mode

; Global Descriptor Table (64-bit).
	GDT64:
	.Null equ $ - GDT64
	dq 0x0000000000000000
	.Code equ $ - GDT64
	dq 0x0020980000000000                   
	.Data equ $ - GDT64
	dq 0x0000900000000000                   
	.Pointer:
	dw $ - GDT64 - 1 			; 16-bit Size (Limit)
	dq GDT64 					; 64-bit Base Address

; World of 64 bits starts here
bits 64

%include "loadelf.asm"

Stage4: 
	cli 

; Set segment registers to the data descriptor.
	mov ax, GDT64.Data			
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov rsp, 0x9000         ; stack starts at 36kb
	
	mov rbx, qword kernel   ; 0x5000 + stage2 offset, start of kernel ELF
	call loadelf

	mov r12, rbx
	xchg bx, bx             ; debugger trap
	mov rdi, 0x0004BEEF     ; integer/pointer for first arg of kernel
	call r12                ; call kernel

	hlt


fileName			db "KERNEL     "
loading 			db "Loading kernel...", 13, 10, 0
preparing 			db "Preparing to launch kernel...", 13, 10, 0
kernelNotFound		db "Can't find kernel!", 13, 10, 0

absolute stage1+512

%include "fat32_volumeid.asm"