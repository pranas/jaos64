; Resources:
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
; also you can use the same code for floppy loaders and floppy-emulated USB thumb drives

; TODO: check BIOS for extension and print error if it's not supported

; For now we will try to work on partition 1

; Set-up address packet to read 1st sector of 1st partition
;4.2.9 http://bochs.sourceforge.net/doc/docbook/user/bochsrc.html
;http://bochs.sourceforge.net/doc/docbook/user/using-bximage.html
;http://www.ugrad.physics.mcgill.ca/wiki/index.php/Preparing_a_Debian_disk_image_for_Bochs
;http://www.andremiller.net/content/mounting-hard-disk-image-including-partitions-using-linux

; setup stack
	mov sp, 0x9000
	mov bp, sp
	
	push dx ; dl should be boot drive?

; prepare to read 1st partition's VolumeID sector
	
	mov eax, [PartitionTable.lba]
	mov [AddressPacket.lba], eax
	
	mov si, AddressPacket
	mov ah, 0x42			; to read?
	mov dl, 0x80			; it's typically C drive, TODO: implement checking
	int 0x13
	jc start				; error

; Now we have first sector of first partition at 0x7e00

; read as much sectors as there are in cluster
	
	xor eax, eax
	mov al, [FAT32.sectors_per_cluster]
	mov [AddressPacket.sectors], ax
	
	mul WORD [FAT32.bytes_per_sector]

; clutser_size = sectors_per_cluster * bytes_per_sector
	push eax
	
; read cluster to 0x9000

	mov WORD [AddressPacket.buffer], 0x9000

; cluster_begin_lba = PartitionTable.lba + fat32.reserved_sectors + 
;					+ (fat32.number_of_fats * fat32.sectors_per_fat2);

	
	mov ebx, [AddressPacket.lba] 		; currently we have there first sector of partition
	add bx, [FAT32.reserved_sectors]

	push ebx ; fat_begin_lba = partition_lba + fat32.reserved_sectors
	mov eax, [FAT32.sectors_per_fat2]
	mov cl, BYTE [FAT32.number_of_fats]
	mul ecx
	
	add ebx, eax

	push ebx	; cluster_begin_lba

;lba_addr = cluster_begin_lba + (cluster_number - 2) * sectors_per_cluster;

	mov eax, [FAT32.root_cluster]
	push eax
	sub eax, 0x02
	mov cl, BYTE [FAT32.number_of_fats]
	mul ecx
	;mul BYTE [FAT32.sectors_per_cluster]
	add eax, ebx
	; eax = lba_addr
	mov [AddressPacket.lba], eax
	mov ah, 0x42
	mov dl, 0x80
	int 0x13
	
	; we have saved some interresting stuff
	; let's label it
	
	%define	boot_drive 			bp-2
	%define	bytes_per_cluster 	bp-6
	%define	fat_begin_lba		bp-10
	%define	cluster_begin_lba	bp-14
	%define current_root		bp-18
	; FATEoFMask = FATClusterMask & 0xFFFFFFF8
	; FATClusterMask = 0x0FFFFFFF
	
; lets travel through root directory and look for our stage2
searchFile:
	xchg bx, bx
	mov ax, 0x9000-32
	mov bx, [bytes_per_cluster]
	add bx, 0x9000

; bx = end of cluster in memory

nextEntry:
	add ax, 32
	
	cmp ax, bx
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
	
	; read cluster
	;lba_addr = cluster_begin_lba + (cluster_number - 2) * sectors_per_cluster;
	sub eax, 0x02
	xor ecx, ecx
	mov cl, BYTE [FAT32.sectors_per_cluster]
	mul ecx
	add eax, [cluster_begin_lba]
	mov WORD [AddressPacket.buffer], 0x5000
	mov [AddressPacket.lba], eax
	mov si, AddressPacket
	mov ah, 0x42
	mov dl, 0x80
	int 0x13
	jmp 0x5000

notFound:
	mov eax, [current_root]
	; call a procedure to get next cluster no
	call getNextCluster
	; if returned something
	cmp eax, 0
	jne searchFile

;
; Procedure to find next cluster's number
;
;	IN: 
;		eax - current cluster
;	OUT:
;		eax - next cluster, 0 if there isn't one

getNextCluster:
	; we need to get FAT entry of this cluster

	add eax, 1
	
	; do the math to find out wich FAT sector to read
	
	
	

	;push eax
	;xor eax, eax
	;mov ax, [FAT32.bytes_per_sector]
	;div 32
	;div ebx
	; edx remainder
	;push edx
	
	; now read that sector
	
	add eax, [fat_begin_lba]
	mov [AddressPacket.lba], eax
	mov eax, 1
	mov [AddressPacket.sectors], eax
	mov si, AddressPacket
	mov ah, 0x42
	mov dl, 0x80
	int 0x13
	
	; now let's do the math again to find out wich entry of FAT do we need
	; (use remainder)
	
	; check entry and prepare output
	
	ret
	
fileName:		db "STAGE2     "
notSupported:	db "Your system is not supported!", 0
unexpectedE:	db "Unexpected error occurred...", 0

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