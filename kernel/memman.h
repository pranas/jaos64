#ifndef _MEMMAN_H
#define _MEMMAN_H

/*

	memman.h

	Memory manager

	Copyright (C) 2010

	This is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2, or (at your option)
	any later version.

*/

#include <stdint.h>
#include <bootinfo.h>

#define MEM_BLOCKS_PER_BYTE 8
#define MEM_BLOCK_SIZE	4096
#define MEM_BLOCK_ALIGN	MEM_BLOCK_SIZE

static	uint64_t	_mem_memory_size=0;
static	uint64_t	_mem_used_blocks=0;
static	uint64_t	_mem_max_blocks=0;
// memory map bit array. Each bit represents a memory block
static	uint64_t*	_mem_memory_map= 0;

/*
	Format of entry in BIOS memory map
*/
struct memory_region {
	//uint32_t	startLo;
	//uint32_t	startHi;
    uint64_t	start;
	//uint32_t	sizeLo;
	//uint32_t	sizeHi;
    uint64_t	size;
	uint32_t	type;
	uint32_t	acpi3;
} __attribute__((packed));
typedef struct memory_region memory_region;

void memman_init (multiboot_info*);

#endif