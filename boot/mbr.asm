
;http://wiki.osdev.org/ATA_in_x86_RealMode_(BIOS)
;http://wiki.osdev.org/Partition_Table
;http://www.pjrc.com/tech/8051/ide/fat32.html

bits 16
org 0x7c00

start:

; I'm tired of all that backward-compatibility issues made by stupid engineers
; BIOSes since mid-90's have extensions to use LBA addressing
; We will depend on that
;
; CHS is better for compatibility, 
; also you can use the same code for floppy loaders and floppy-emulated USB thumb drives)

; TODO: check BIOS for extension and print error if it's not supported

; For now we will try to work on partition 1

; Set-up address packet to read 1st sector of 1st partition
;4.2.9 http://bochs.sourceforge.net/doc/docbook/user/bochsrc.html
;http://bochs.sourceforge.net/doc/docbook/user/using-bximage.html
;http://www.ugrad.physics.mcgill.ca/wiki/index.php/Preparing_a_Debian_disk_image_for_Bochs
;http://www.andremiller.net/content/mounting-hard-disk-image-including-partitions-using-linux

	;mov si, AddressPacket.sectors
	;mov WORD [si], 0x01					; 1 sector to read
	;mov si, PartitionTable.lba
	;mov bx, [si]
	;add si, 2
	;mov cx, [si]
	;mov si, AddressPacket.lba
	;mov [si], bx
	;add si, 2
	;mov [si], cx
	;mov si, AddressPacket.buffer
	;mov WORD [si], 0x7e00
	
	mov eax, [PartitionTable.lba]
	mov [AddressPacket.lba], eax

	mov si, AddressPacket
	mov ah, 0x42			; to read?
	mov dl, 0x80			; it's typically C drive, TODO: implement checking
	int 0x13
	jc start				; error

; Now we have first sector of first partition at 0x7e00

; read as much sectors as there are in cluster
	
	xor ax, ax
	mov al, [FAT32.sectors_per_cluster]
	mov [AddressPacket.sectors], ax
	
; read cluster to 0x8000

	mov WORD [AddressPacket.buffer], 0x8000
	
;(unsigned long)cluster_begin_lba = Partition_LBA_Begin + Number_of_Reserved_Sectors + (Number_of_FATs * Sectors_Per_FAT);
	
	mov ebx, [AddressPacket.lba] 		; currently we have there first sector of partition
	add ebx, [FAT32.reserved_sectors] 
	;add [FAT32.reserved_sectors], ebx
	mov eax, [FAT32.sectors_per_fat2]
	mul WORD [0x0000 + FAT32.number_of_fats]
	
	add ebx, eax

;lba_addr = cluster_begin_lba + (cluster_number - 2) * sectors_per_cluster;

	mov eax, [FAT32.root_cluster]
	sub eax, 0x02
	mul BYTE [FAT32.sectors_per_cluster]
	add eax, ebx
	mov [AddressPacket.lba], eax
	mov ah, 0x42
	mov dl, 0x80
	int 0x13
	

; FAT_begin_lba = AddressPacket.lba + FAT32.reserved_sectors;
; 
;mov si, AddressPacket.lba
;mov [si], bx
;mov si, FAT32.reserved_sectors
;mov [si], ax
;add ax, bx				; ax = begining of file allocation table

;(unsigned long)fat_begin_lba = Partition_LBA_Begin + Number_of_Reserved_Sectors;
;(unsigned long)cluster_begin_lba = Partition_LBA_Begin + Number_of_Reserved_Sectors + (Number_of_FATs * Sectors_Per_FAT);
;(unsigned char)sectors_per_cluster = BPB_SecPerClus;
;(unsigned long)root_dir_first_cluster = BPB_RootClus;

AddressPacket:
.size			db 16
				db 0 ; always zero, because of specification?
.sectors		dw 0x0001
.buffer			dw 0x7e00
				dw 0
.lba			dd 0
.lba48			dd 0 ; used by 48 bit LBAs

; Boot code size should be 446 bytes
; If we managed to use less pad everything with zeros

times 446 - ($-$$) db 0

; Everything from here actually won't be used
; it's just to have labels for easier referencing

PartitionTable:
.boot_flag		db 0x80 ;means active, TODO: check for bootable partition
.begin_chs		db 0xff
				db 0xff
				db 0xff
.type			db 0xff
.end_chs 		db 0xff
				db 0xff
				db 0xff
.lba			dd 0xffffffff
.sectors 		dd 0xffffffff
.part2			times 16 db 0xff
.part3			times 16 db 0xff
.part4			times 16 db 0xff
.sig			dw 0xAA55

; VolumeID sector will be loaded here at 0x7e00
; (basically it's 1st sector of 1st partition)

db 0xf	;jmp over_fat_table
db 0xf
db 0xf	;nop

;
; We will make labeled FAT boot record here
; We will only use those labels as memory references
;
; It's for easier referencing to these fields
; from our MBR loader
;

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
.reserved				dd 0
						dd 0
						dd 0
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