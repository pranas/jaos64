bits 16
org 0x7c00

start:
	jmp loader

; simple print procedure using bios interrupts
%include "print.inc"
	
loader:
; clear segments to 0
	xor ax, ax
	mov ds, ax
	mov es, ax

; print welcome message
	mov si, LoaderStartStr
	call Print

; reset floppy
FReset:
	mov si, ResetFStr
	call Print

	mov ah, 0
	mov dl, 0
	int 0x13
	jc FReset ; if reset failed try again
; end floppy reset

	mov si, ReadFStr
	call Print

; prepare segment regs for reading
	mov ax, 0x0500
	mov es, ax
	xor bx, bx
	; write into memory address es:bx (0x0500:0)

; start reading more sectors
FRead:
	mov ah, 0x02
	mov al, 1 ; read how many sectors
	mov ch, 0 ; which (cylinder or track ?)
	; still 1st one, and indexation starts from 0 not 1,
	; NO THANKS TO BLACKTHORN
	mov cl, 2 ; starting from which sector (1st was already loaded by BIOS)
	mov dh, 0 ; head number
	mov dl, 0 ; drive number ( 0 - floppy )
	int 0x13
	jc FRead ; in case of error try again
; end reading sectors
	
	mov si, LoadS2Str
	call Print

	jmp 0x0500:0x0 ; jump to stage 2
		
	cli
	hlt

LoaderStartStr db "Starting loader...", 13, 10, 0
ResetFStr db "Resetting floppy...", 13, 10, 0
ReadFStr db "Reading from floppy...", 13, 10, 0
LoadS2Str db "Trying to load Stage2...", 13, 10, 0

times 510 - ($-$$) db 0 ; fill till the end with zeros
dw 0xAA55 ; boot sector magic number for validation, wont work without this
