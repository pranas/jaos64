org 0x0 ; we are actually at 0x1000 right now, no additional offset needed

bits 16

start:
	jmp s2

%include "print.inc"
%include "gdt.inc"

s2:
; ds must be same as cs
	cli
	push cs
	pop ds

; print warm welcome :)
	mov si, S2WelcomeStr
	call Print;

	call loadGDT

; halt processor
	cli
	hlt

S2WelcomeStr db "Welcome to Stage2 :)", 13, 10, 0

times 512 - ($-$$) db 0 ; pad to 512 bytes
