bits 16

loadGDT:
	cli
	pusha
	lgdt [toc]
	sti
	popa
	ret

toc:
	dd (gdt_end - gdt_start - 1) ; size of GDT minus 1
	dw gdt_start           ; base of GDT

gdt_start:
; null descriptor 
	dd 0x00000000
	dd 0x00000000

; code descriptor
	dw 0xFFFF    ; limit low
	dw 0x0000    ; base low
	db 00000000b ; base middle
	db 10011010b ; access
	db 11001111b ; granularity
	db 00000000b ; base high

; data descriptor
	dw 0xFFFF    ; limit low (Same as code)
	dw 0x0000    ; base low
	db 00000000b ; base middle
	db 10010010b ; access
	db 11001111b ; granularity
	db 00000000b ; base high
gdt_end:

%define CODE_DESC 0x8
