#pragma once
#ifndef FAT32_H
#define FAT32_H

#include <stdint.h>
#include <string.h>
#include "memman.h"
#include "kheap.h"
#include "diskio.h"

struct fat32_volume_id {
	char jump[3];
	char oem_identifier[8];
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
	uint32_t reserved[3];
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
	char label[11];
	//System identifier string. Always "FAT32 ".
	//The spec says never to trust the contents of this string for any use.
	char identifier[8];
} __attribute__((packed));

typedef struct fat32_volume_id fat32_volume_id;

struct partition_entry
{
	uint8_t flag;
	uint8_t chs_begin[3];
	uint8_t type;
	uint8_t chs_end[3];
	uint32_t lba;
	uint32_t sectors;
} __attribute__((packed));

typedef struct partition_entry partition_entry;

struct partition_table
{
	char boot_code[446];
	partition_entry partition[4];
	uint16_t magic;
} __attribute__((packed));

typedef struct partition_table partition_table;

struct dir_entry
{
	char filename[11];
	struct
	{
		union
		{
			uint8_t attribute;
			struct {
				uint8_t ro:1;
				uint8_t hidden:1;
				uint8_t sys:1;
				uint8_t volid:1;
				uint8_t dir:1;
				uint8_t archive:1;
				uint8_t unused:2;
			};
		};
	};
	uint8_t nt;
	uint8_t creation_ms;
	uint16_t creation_time;
	uint16_t creation_date;
	uint16_t access_date;
	uint16_t cluster_high;
	uint16_t modification_time;
	uint16_t modification_date;
	uint16_t cluster_low;
	uint32_t size;
} __attribute__((packed));

typedef struct dir_entry dir_entry;

// private
void fat32_init();
uint64_t cluster2lba(uint64_t cluster);
uint64_t read_cluster(uint64_t cluster, void* address);
uint64_t find_next_cluster(uint64_t cluster);
// public
void read_file(uint64_t cluster, void* address);
dir_entry* find_file(char* name);
fat32_volume_id* get_current_partition();
// debug
void put_dir(dir_entry* dir, int size);

#endif