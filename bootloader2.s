bits 16

org 0x0 ; we are actually at 0x0500

start:
	jmp Stage2

%include "print.inc"
%include "gdt.inc"

Stage2:

; clear segments
	cli
	mov ax, cs
	mov ds, ax
	mov es, ax
	mov ax, 0x9000 ; dont know why :D
	mov ss, ax
	mov sp, 0xFFFF
	sti

; print warm welcome :)
	mov si, S2WelcomeStr
	call Print


; A20
;	pusha
;	mov ax, 0x2401
;	int 0x15
;	popa

	call loadGDT

; try to enter protected mode
	cli ; interrupts are deadly to us from now on
	mov eax, cr0
	or  eax, 1
	mov cr0, eax

; jump to code descriptor because CS is wrong now
	jmp 0x8:Stage3 ; 0x8 is the code descriptor offset

; welcome to the kingdom of 32 bits
bits 32

Stage3:

; set other segments to data descriptors
	mov ax, 0x10
	mov ds, ax
	mov ss, ax
	mov es, ax
	mov esp, 0x90000 ; why 0x90000 ? I dont know :D

; halt processor
	cli
	hlt

S2WelcomeStr db "Welcome to Stage2 :)", 13, 10, 0
times (4*512) - ($-$$) db 0 ; pad to 512 bytes
