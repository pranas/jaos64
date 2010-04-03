
Print:
	pusha
.loop:
	lodsb
	or al, al
	jz done
	xor	bx, bx		; A faster method of clearing BX to 0
	mov	ah, 0x0e
	int	0x10
	jmp .loop
done:
	popa
	ret
