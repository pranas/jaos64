bits 64

global gdt_flush
extern gdt_ptr

gdt_flush:
	lgdt [gdt_ptr]
	mov eax, 0x10
	mov ds, eax
	mov es, eax
	mov fs, eax
	mov gs, eax
	mov ss, eax
	jmp flush
flush:
	ret
