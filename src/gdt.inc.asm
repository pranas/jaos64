bits 16

loadGDT:
	cli
	pusha
	lgdt [toc]
	sti
	popa
	ret

gdt_start:
; null descriptor 
	dd 0
	dd 0

; code descriptor
	dw 0FFFFh    ; limit low
	dw 0    ; base low
	db 0 ; base middle
	db 10011010b ; access
	db 11001111b ; granularity
	db 0 ; base high

; data descriptor
	dw 0FFFFh    ; limit low (Same as code)
	dw 0    ; base low
	db 0 ; base middle
	db 10010010b ; access
	db 11001111b ; granularity
	db 0 ; base high
gdt_end:

toc:
	dw gdt_end - gdt_start - 1 ; size of GDT minus 1
	dd gdt_start           ; base of GDT

%define CODE_DESC 0x8
