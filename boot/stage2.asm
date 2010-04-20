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
    mov ax, 0x0
    mov es, ax
    mov di, 512
    mov [bootinfo.mmap_addr], di
    pushad
    call BiosGetMemoryMap
    mov [bootinfo.mmap_length], ebp ; count of entries in map
	popad
	
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
	mov esp, stack-22 ;0x90000 ; why 0x90000 ? I dont know :D
	
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

; We will use paging to set-up higher half kernel addressing
;
; We will put our kernel at 3GB in our virtual address space
;
; 0x00000000 c0000000
;
; Bits 63-48 are sign extension as required for canonical-address forms
; Bits 47-39 index into PML4T -> in this case it's PML4T[0]
; Bits 38-30 index into PDPT -> ... PDPT[3]
; Bits 29-21 ... PDT
; Bits 20-12 ... PT
; Bits 11-0 offset in page
;
	mov di, 0x1000
	mov WORD [di], 0x2003
	
	mov di, 0x2000
	mov WORD [di], 0x3003
	
	add di, 0x18            ;3gb
	mov WORD [di], 0x5003
	
	mov di, 0x3000
	mov WORD [di], 0x4003
	
	mov di, 0x5000
	mov WORD [di], 0x6003

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
	
; This will map our 3GB mem to second MB on physical memory (0xc0000000 -> 0x1FFFFF)

    mov di, 0x6000				; Our PT starts there
    mov ebx, 0x00200003			; 3 again to set first two bits
    mov cx, 512					; Loop

    .setPageEntry2:
    mov DWORD [di], ebx
    add ebx, 0x1000
    add di, 8					; Move to next page entry
    loop .setPageEntry2

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

; Set segment registers to the data descriptor.
	mov ax, GDT64.data			
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov rsp, stack-22; 0x90000         ; stack starts at 36kb
;
; Let's look for kernel file in disk
;
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
	
;
; Call kernel
;

	xchg bx, bx                 ; debugger trap
	mov r12, rbx
	mov rdi, bootinfo           ; integer/pointer for first arg of kernel
	call r12                    ; call kernel

	hlt


fileName			db "KERNEL     "
loading 			db "Loading kernel...", 13, 10, 0
preparing 			db "Preparing to load kernel...", 13, 10, 0

bootinfo:
.flags				dd	0	; required
.memoryLo			dd	0	; memory size. Present if flags[0] is set
.memoryHi			dd	0
.bootDevice			dd	0	; boot device. Present if flags[1] is set
.cmdLine			dd	0	; kernel command line. Present if flags[2] is set
.mods_count			dd	0	; number of modules loaded along with kernel. present if flags[3] is set
.mods_addr			dd	0
.syms0				dd	0	; symbol table info. present if flags[4] or flags[5] is set
.syms1				dd	0
.syms2				dd	0
.mmap_length		dd	0	; memory map. Present if flags[6] is set
.mmap_addr			dd	0
.drives_length		dd	0	; phys address of first drive structure. present if flags[7] is set
.drives_addr		dd	0
.config_table		dd	0	; ROM configuation table. present if flags[8] is set
.bootloader_name 	dd	0	; Bootloader name. present if flags[9] is set
.apm_table			dd	0	; advanced power management (apm) table. present if flags[10] is set
.vbe_control_info 	dd	0	; video bios extension (vbe). present if flags[11] is set
.vbe_mode_info		dd	0
.vbe_mode			dw	0
.vbe_interface_seg 	dw	0
.vbe_interface_off 	dw	0
.vbe_interface_len 	dw	0

absolute stage1+512

%include "fat32_volumeid.asm"
