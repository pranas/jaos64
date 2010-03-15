;
; FAT32 VolumeID sector for referencing
;

resb 2	;jmp over_fat_table
resb 1	;nop

FAT32:
.oem_identifier			resq 1
.bytes_per_sector		resw 1
.sectors_per_cluster	resb 1
.reserved_sectors		resw 1
.number_of_fats			resb 1
; Number of directory entries 
; (must be set so that the root directory occupies entire sectors)
.directory_entries		resw 1
; The total sectors in the logical volume
; If this value is 0, it means there are more than 65535 sectors in the volume
; and the actual count is stored in "Large Sectors (bytes 32-35)
.total_sectors			resw 1
.media_type				resb 1
; FAT12/FAT16 only
.sectors_per_fat		resw 1
.sectors_per_track		resw 1
.number_of_heads		resw 1
.hidden_sectors			resd 1
.large_sectors			resd 1
.sectors_per_fat2		resd 1
; Bits 0-3 = Active FAT, 7 = !FAT mirroring
.flags					resw 1
.version 				resw 1
.root_cluster 			resd 1
.fsinfo					resw 1
.backup_boot			resw 1
.reserved				resd 3
; The values here are identical to the values returned by the 
; BIOS interrupt 0x13
; 0x00 for a floppy disk
; 0x80 for hard disks
.drive_number			resb 1
; Flags in windows NT. Reserved otherwise.
.ntflags				resb 1
; Signature (must be 0x28 or 0x29)
.signature 				resb 1
.volume_id				resd 1
; 11-bytes volume label padded with spaces
.label 					resb 11
; System identifier string. Always "FAT32 ".
; The spec says never to trust the contents of this string for any use.
.identifier				resq 1