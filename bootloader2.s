S2WelcomeStr db "Welcome to Stage2 :)", 13, 10, 0
org 0x0 ; we are actually at 0x1000 right now, no additional offset needed

bits 16

start:
	jmp s2
;%include "print.inc"
Print:
	lodsb
	or al, al
	jz done
	xor	bx, bx		; A faster method of clearing BX to 0
	mov	ah, 0x0e
	int	0x10
	jmp Print
done:
	ret
s2:
; ds must be same as cs
	cli
	push cs
	pop ds
;	mov ax, cs
;	mov ds, ax

; print warm welcome :)
	mov si, S2WelcomeStr
	call Print

; halt processor
	cli
	hlt

; S2WelcomeStr db "Welcome to Stage2 :)", 13, 10, 0

times 512 - ($-$$) db 0 ; pad to 512 bytes
