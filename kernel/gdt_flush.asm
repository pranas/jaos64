bits 64

global gdt_flush
extern gdt_ptr

gdt_flush:
	xchg bx, bx
	mov rbx, gdt_ptr
	lgdt [rbx]
	mov eax, 0x10
	mov ds, eax
	mov es, eax
	mov fs, eax
	mov gs, eax
	mov ss, eax
	jmp flush
flush:
	ret
