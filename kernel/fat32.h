#ifndef _FAT32_H
#define _FAT32_H

#include <stdint.h>

struct fat32_volume_id {
	char[3] jump;
	char[8] oem_identifier;
	uint16_t bytes_per_sector;
	uint8_t sectors_per_cluster;
	uint16_t reserved_sectors;
	uint8_t number_of_fats;
	uint16_t directory_entries;
	uint16_t total_sectors; // used if < 65535
	uint8_t media_type;
	// FAT12/FAT16 only
	uint16_t sectors_per_fat;
	uint16_t sectors_per_track;
	uint16_t number_of_heads;
	uint32_t hidden_sectors;
	uint32_t large_sectors; // used if > 65535
	// FAT32 specific
	uint32_t sectors_per_fat2;
	// Flags: Bits 0-3 = Active FAT, 7 = !FAT mirroring
	uint16_t flags;
	uint16_t version;
	uint32_t root_cluster;
	uint16_t fsinfo;
	uint16_t backup_boot;
	uint32_t[3] reserved;
	//The values here are identical to the values returned by the 
	//BIOS interrupt 0x13
	//0x00 for a floppy disk
	//0x80 for hard disks
	uint8_t drive_number;
	//Flags in windows NT. Reserved otherwise.
	uint8_t ntflags;
	//Signature (must be 0x28 or 0x29)
	uint8_t signature;
	uint32_t volume_id;
	//11-bytes volume label padded with spaces
	char[11] label;
	//System identifier string. Always "FAT32 ".
	//The spec says never to trust the contents of this string for any use.
	char[8] identifier;
} __attribute__((packed));

typedef struct fat32_volume_id fat32_volume_id;

#endif