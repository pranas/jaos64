[bits 64]

; 0x20 - ring 3 data segment
; 0x18 - ring 3 code segment
[global switch_to_user_mode]
switch_to_user_mode:
	cli
	mov ax, 0x23
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	
	mov rax, 0xc0000000
	push 0x23 ; data segment
	push rax  ; stack pointer
	pushf     ; flags
	pop rax
	or rax, 0x200 ; enable interrupt flag
	push rax
	push 0x1B ; code segment
	push rdi  ; instruction pointer (passed as parameter by rdi)
	iretq     ; try to jump
