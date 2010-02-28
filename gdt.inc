bits 16

loadGDT:
	pusha
	cli
	lgdt [toc]
	sti
	popa
	ret

; null descriptor 
nulld	dd 0
		dd 0 

; code descriptor
coded	dw 0FFFFh    ; limit low
		dw 0         ; base low
		db 0         ; base middle
		db 10011010b ; access
		db 11001111b ; granularity
		db 0         ; base high

; data descriptor
datad	dw 0FFFFh    ; limit low (Same as code)
		dw 0         ; base low
		db 0         ; base middle
		db 10010010b ; access
		db 11001111b ; granularity
		db 0         ; base high

end:
toc:
	dw end - nulld - 1 ; size of GDT - 1
	dd nulld           ; base of GDT
