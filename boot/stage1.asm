;
; I just understood that everything i was reading were talking about 
; first sector of partition
;
; As i think now, boot code there should be loaded by a loader in MBR
;
; 


bits 16
org 0x7c00

start:
	jmp stage1
	nop
	
;
; We will make labeled FAT boot record here
;
; This will let us to use those labels as references to FAT fields
FAT32:
.oem_identifier			dq 1684300916843009
.bytes_per_sector		dw 257
.sectors_per_cluster	db 1
.reserved_sectors		dw 257
.number_of_fats			db 1
; Number of directory entries 
; (must be set so that the root directory occupies entire sectors)
.directory_entries		dw 257
; The total sectors in the logical volume
; If this value is 0, it means there are more than 65535 sectors in the volume
; and the actual count is stored in "Large Sectors (bytes 32-35)
.total_sectors			dw 257
.media_type				db 1
; FAT12/FAT16 only
.sectors_per_fat		dw 257
.sectors_per_track		dw 257
.number_of_heads		dw 257
.hidden_sectors			dd 16843009
.large_sectors			dd 16843009
.sectors_per_fat2		dd 16843009
; Bits 0-3 = Active FAT, 7 = !FAT mirroring
.flags					dw 257
.version 				dw 257
.root_cluster 			dd 16843009
.fsinfo					dw 257
.backup_boot			dw 257
.reserved				dd 16843009
						dd 16843009
						dd 16843009
; The values here are identical to the values returned by the 
; BIOS interrupt 0x13
; 0x00 for a floppy disk
; 0x80 for hard disks
.drive_number			db 1
; Flags in windows NT. Reserved otherwise.
.ntflags				db 1
; Signature (must be 0x28 or 0x29)
.signature 				db 1
.volume_id				dd 16843009
; 11-bytes volume label padded with spaces
.label 					dd 16843009
						dd 16843009
						dw 257
						db 1
; System identifier string. Always "FAT32 ".
; The spec says never to trust the contents of this string for any use.
.identifier				dq 1684300916843009

; Now we have exactly 420 bytes left to load our second stage
stage1:
	jmp loader2

; simple print procedure using bios interrupts
%include "print.inc.asm"
	
loader2:
; set segments
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
	; write into memory address es:bx (0x0:0x5000)

; start reading more sectors
FRead:
	mov ah, 0x02
	mov al, 4 ; read how many sectors
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

; 0xAA55 bootable partition signature.
dw 0xAA55
