;
; FAT32 stage1 bootloader
;
; Resources used:
;	http://www.pjrc.com/tech/8051/ide/fat32.html
;	http://wiki.osdev.org/ATA_in_x86_RealMode_(BIOS)
;	http://wiki.osdev.org/Partition_Table
;	http://bochs.sourceforge.net/doc/docbook/user/bochsrc.html (4.2.9)
;	http://bochs.sourceforge.net/doc/docbook/user/using-bximage.html
;	http://www.ugrad.physics.mcgill.ca/wiki/index.php/Preparing_a_Debian_disk_image_for_Bochs
;	http://www.andremiller.net/content/mounting-hard-disk-image-including-partitions-using-linux

%include "config.asm"

bits 16
org stage1

; I'm tired of all that backward-compatibility issues made by stupid engineers
; BIOSes since mid-90's have extensions to use LBA addressing
; We will depend on that
;
; TODO:
;	check for bootable partition (for now we're working only on partition 1)

; Setup stack
	mov sp, 0x9000
	mov bp, sp
	
	push dx ; dl should be boot drive

; check if BIOS extension for LBA addressing is supported
	mov ah, 0x41
	mov bx, 0x55aa
	mov dl, 0x80
	int 0x13
	mov si, notSupported
	jnc supported
	jmp print
	
supported:

; Prepare to read 1st partition's VolumeID sector
	
	mov eax, [PartitionTable.lba]
	mov [AddressPacket.lba], eax
	
	mov si, AddressPacket
	call readDisk

; Now we have first sector of first partition at 0x7e00
;
; Prepare to read root directory's cluster
;
; Read as much sectors as there are in cluster
	
	xor eax, eax
	mov al, [FAT32.sectors_per_cluster]
	mov [AddressPacket.sectors], ax
	
	mul WORD [FAT32.bytes_per_sector]

; clutser_size = sectors_per_cluster * bytes_per_sector
	push eax
	
; read cluster to 0x9000
	mov WORD [AddressPacket.buffer], bp	; = 0x9000

; cluster_begin_lba = PartitionTable.lba + FAT32.reserved_sectors + 
;					+ (FAT32.number_of_fats * FAT32.sectors_per_fat2);

	
	mov ebx, [AddressPacket.lba] 		; currently it's PartitionTable.lba
	add bx, [FAT32.reserved_sectors]
; Saving fat_begin_lba (= partition_lba + fat32.reserved_sectors)
	push ebx
	mov eax, [FAT32.sectors_per_fat2]
	mov cl, BYTE [FAT32.number_of_fats]
	mul ecx
	add ebx, eax
	push ebx							; cluster_begin_lba
	mov eax, [FAT32.root_cluster]
	push eax 							; current_cluster
	call clusterLBA
	mov [AddressPacket.lba], eax
	call readDisk
	
	; we have saved some interresting stuff
	; let's label it
	
	%define	boot_drive 			bp-2
	%define	bytes_per_cluster 	bp-6
	%define	fat_begin_lba		bp-10
	%define	cluster_begin_lba	bp-14
	%define current_cluster		bp-18

	
; lets travel through root directory and look for our file to load
searchFile:
	mov ax, 0x9000-32
	mov bx, [bytes_per_cluster]
	add bx, bp ;0x9000

nextEntry:
	add ax, 32
	cmp ax, bx		; bx = end of cluster in memory
	je notFound

	mov di, fileName
	mov si, ax
	mov cx, 11
	; compare names
	repe cmpsb
	; if not the same, try next entry
	jnz nextEntry
	
	; file found
	
	; File entry looks like this:
	;
	; Filename 		times 11 db
	; Attribute 	db
	; ____ 			dq
	; Cluster High 	dw
	; ____ 			dd
	; Cluster Low 	dw
	; Size 			dd
	
	mov ax, [si+9]			; si is already +11 in entry
	push ax					; push low word
	mov ax, [si+15]			
	push ax					; push high word
	pop eax 				; eax = 1st cluster of file
	
	mov [current_cluster], eax
	call clusterLBA
	
	mov WORD [AddressPacket.buffer], stage2
	mov [AddressPacket.lba], eax
	mov si, AddressPacket
	call readDisk

; load more if there is

oneMoreCluster:	
	xor ebx, ebx
	mov eax, [AddressPacket.buffer]
	mov bx, [bytes_per_cluster]
	add eax, ebx
	mov [AddressPacket.buffer], eax

	call getNextCluster
	jnc oneMoreCluster		; not EOF
	
	; we loaded our file completely
	jmp stage2

notFound:
	; call a procedure to get next cluster
	call getNextCluster
	; if returned something
	jnc searchFile
	
	; File not found!
	mov si, fileNotFound
	jmp print
	
;
; clusterLBA, readDisk
;

%include "diskio.asm"

;
;	Procedure prints buffer to monitor
;		IN:
;			ds:si - pointer to buffer
;		

print:
	.loop:
	lodsb
	or al, al
	jz done
	xor	bx, bx
	mov	ah, 0x0e
	int	0x10
	jmp .loop
	done:
	hlt
	
fileName:		db "STAGE2     "
fileNotFound:	db "Loader not found!", 0
notSupported:	db "Your system is not supported!", 0

AddressPacket:
.size			db 16
				db 0 			; always zero, because of specification?
.sectors		dw 0x0001
.buffer			dw 0x7e00
				dw 0
.lba			dd 0
.lba48			dd 0 			; used by 48 bit LBAs

; Boot code size should be 446 bytes
; If we managed to use less pad everything with zeros

times 446 - ($-$$) db 0

; Everything from here is actually
; just labels for easier referencing

absolute stage1+446

PartitionTable:
.boot_flag		resb 1; 0x80 means active, TODO: check for bootable partition
.begin_chs		resb 3
.type			resb 1
.end_chs 		resb 3
.lba			resd 1
.sectors 		resd 1
.part2			resb 16
.part3			resb 16
.part4			resb 16
;.sig			dw 0xAA55

; VolumeID sector will be loaded at 0x7e00 (stage1 + 512)

absolute stage1+512

%include "fat32_volumeid.asm"