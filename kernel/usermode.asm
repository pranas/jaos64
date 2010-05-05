[bits 64]

; 0x20 - ring 3 data segment
; 0x18 - ring 3 code segment

usermode:
	mov ax, 0x20
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	
	mov rax, rsp
	push 0x20
	push rax
	pushf
	push 0x18
	push here	
	iretq
here:
