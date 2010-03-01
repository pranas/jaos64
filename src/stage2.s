bits 16

org 0x5000 ; we are actually at 0x5000

start:
	jmp Stage2

%include "print.inc"
%include "gdt.inc"

Stage2:

; clear segments
	cli
	xor ax, ax
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov sp, 0xFFFF
	sti

; print warm welcome :)
	mov si, S2WelcomeStr
	call Print

; A20
	pusha
	mov ax, 0x2401
	int 0x15
	popa

	call loadGDT

; try to enter protected mode
;lb 0x5058
	cli ; interrupts are deadly to us from now on
	mov eax, cr0
	or  eax, 1
	mov cr0, eax
	
; jump to code descriptor because CS is wrong now
	jmp 0x08:Stage3 ; 0x8 is the code descriptor offset

; welcome to the kingdom of 32 bits
bits 32

Stage3:

; set other segments to data descriptors
	mov ax, 0x10 ;0x0000506a
	mov ds, ax
	mov ss, ax
	mov es, ax
	mov esp, 0x90000 ; why 0x90000 ? I dont know :D
	
; Prepare 64bit stuff

; We will put our paging tables from 0x1000 to 0x5000 but
; first we will clear those tables

; PML4T - 0x1000
; PDPT - 0x2000
; PDT - 0x3000
; PT - 0x4000

	; Set the destination index to 0x1000.
    mov di, 0x1000
    ; Nullify the A-register.
    xor ax, ax
    ; Set the C-register to 4000h (16384).
    mov cx, 0x4000
    ; Clear the memory.
    rep stosb

; Lets make PML4T[0] point to the PDPT and so on:
	mov di, 0x1000
	; Set the word at the destination index to 0x2003.
	mov WORD [di], 0x2003
	; Add 0x1000 to the destination index.
	add di, 0x1000
	; Set the word at the destination index to 0x3003.
	mov WORD [di], 0x3003
	; Add 0x1000 to the destination index.
	add di, 0x1000
	; Set the word at the destination index to 0x4003.
	mov WORD [di], 0x4003
	; Add 0x1000 to the destination index.
	add di, 0x1000

; If you haven't noticed already, I used a three
; This simply means that the first two bits should be set.

; Now all that's left to do is identity map the first two megabytes:

	mov di, 0
	; Set the destination index to 0x4000.
	add di, 0x4000
	; Set the B-register to 0x00000003.
	mov ebx, 0x00000003
	; Set the C-register to 512.
	mov cx, 512

	.SetEntry:
	; Set the double word at the destination index to the B-register.
	mov DWORD [di], ebx
	; Add 0x1000 to the B-register.
	add ebx, 0x1000
	; Add eight to the destination index.
	add di, 8
	; Set the next entry.
	loop .SetEntry

;Now we should enable PAE-paging by setting the PAE-bit in the fourth control register:

	; Set the A-register to control register 4.
	mov eax, cr4
	; Set the PAE-bit, which is the 6th bit.
	or eax, 00000000000000000000000000100000b
	; Set control register 4 to the A-register.
	mov cr4, eax

; And we should set the third control register to the PML4T:
	mov eax, 0
	; Set the A-register to 0x00004000.
	or eax, 0x00001000 ;or eax, 0x00001000 ; 
     ; Set control register 3 to the A-register.
    mov cr3, eax

;Now paging is set up, but it isn't enabled yet.
;We should set the long mode bit in the EFER MSR

	; Set the C-register to 0xC0000080, which is the EFER MSR.
    mov ecx, 0xC0000080
    ; Read from the model-specific register.
    rdmsr
    ; Set the LM-bit which is the 9th bit.
    or eax, 00000000000000000000000100000000b
    ; Write to the model-specific register.
    wrmsr

;Enabling paging

	; Set the A-register to control register 0.
	mov eax, cr0
	; Set the PG-bit, which is the 32nd bit.
	or eax, 10000000000000000000000000000000b
	; Set control register 0 to the A-register.
	mov cr0, eax

;Now we're in compatibility mode.

	; Load the 64-bit global descriptor table.
    lgdt [GDT64.Pointer]

    ; Set the code segment and enter 64-bit long mode.
    jmp GDT64.Code:Realm64

; halt processor
;	cli
;	hlt

; Global Descriptor Table (64-bit).
GDT64:
    ; The null descriptor.
    .Null equ $ - GDT64
     ; Limit (low).
    dw 0
     ; Base (low).
    dw 0
    ; Base (middle)
    db 0
    ; Access.
    db 0
    ; Granularity.
    db 0
    ; Base (high).
    db 0
    ; The code descriptor.
    .Code equ $ - GDT64
    ; Limit (low).
    dw 0FFFFh
    ; Base (low).
    dw 0
    ; Base (middle).
    db 0
    ; Access.
    db 10011000b
    ; Granularity.
    db 00100000b
    ; Base (high).
    db 0
    ; The data descriptor.
    .Data equ $ - GDT64
    ; Limit (low).
    dw 0FFFFh
    ; Base (low).
    dw 0
    ; Base (middle).
    db 0
    ; Access.
    db 10010000b
    ; Granularity.
    db 00000000b
    ; Base (high).
    db 0
    ; The GDT-pointer.
    .Pointer:
    ; Limit.
    dw $ - GDT64 - 1
    ; Base.
    dq GDT64

S2WelcomeStr db "Welcome to Stage2 :)", 13, 10, 0

; world of 64bits starts here
bits 64

Realm64:
    ; Clear the interrupt flag.
    cli
    ; Set the A-register to the data descriptor.
    mov ax, GDT64.Data
    ; Set the data segment to the A-register.
    mov ds, ax
    ; Set the extra segment to the A-register.
    mov es, ax
    ; Set the F-segment to the A-register.
    mov fs, ax
    ; Set the G-segment to the A-register.
    mov gs, ax
    ; Set the destination index 0x00000000000B8000.
    mov rdi, 0x00000000000B8000
    ; Set the A-register to 0x1F201F201F201F20.
    mov rax, 0x1F201F201F201F20
    ; Set the C-register to 500.
    mov rcx, 500
    ; Clear the screen.
    rep movsq
    ; Halt the processor.
    hlt

times (4*512) - ($-$$) db 0 ; pad to 512 bytes