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
#define MEM_BYTES_PER_WORD 8
#define MEM_BLOCK_SIZE	4096
#define MEM_BLOCK_ALIGN	MEM_BLOCK_SIZE
#define MEM_BITMAP 0x20000
#define MEM_DEBUG 1

static uint64_t	_mem_memory_size=0;
static uint64_t	_mem_used_blocks=0;
static uint64_t _mem_max_blocks=0;
static uint64_t* _mem_memory_map = 0;

// private functions to work with memory bitmap
inline void mmap_set (uint64_t bit);
inline void mmap_unset (uint64_t bit);
inline uint64_t mmap_test (uint64_t bit);
uint64_t mmap_first_free();
uint64_t mmap_first_free_zone(uint64_t size);

// public api of memory manager
void memman_init (multiboot_info*);
void mem_init_region(uint64_t base, uint64_t size);
void* mem_alloc_block();
void mem_free_block(void* physical_address);
void* mem_alloc_blocks(uint64_t size);
void mem_free_blocks(void* physical_address, uint64_t size);
uint64_t mem_free_block_count();

// debug procedures
void debug_memmap(uint64_t blocks);

// format of entry in BIOS memory map
struct memory_region
{
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

// format of virtual memory address
struct virtual_addr
{
    uint64_t physical_offset: 12;
    uint64_t page_table: 9;
    uint64_t page_directory: 9;
    uint64_t directory_pointer: 9;
    uint64_t pml4: 9;
    uint64_t sign: 16;
} __attribute__((packed));

typedef struct virtual_addr virtual_addr;

// format of physical memory address
struct physical_addr
{
    uint64_t offset: 12;
    // uint64_t page_frame: 52;
    uint64_t page_frame: 36;
    uint64_t sign: 16;
} __attribute__((packed));

typedef struct physical_addr physical_addr;

struct pml4_entry
{
    uint64_t present : 1;   // Page present in memory
    uint64_t rw : 1;   // Read-only if clear, readwrite if set
    uint64_t user : 1;   // Supervisor level only if clear
    uint64_t pwt : 1;
    uint64_t pcd : 1;
    uint64_t accessed : 1;   // Has the page been accessed since last refresh?
    uint64_t ignored : 1;
    uint64_t mbz : 2;
    uint64_t avl : 3;
    uint64_t directory_pointer : 20;
    uint64_t reserved : 11;
    uint64_t nx : 1;
} __attribute__((packed));

typedef struct pml4_entry pml4_entry;

struct pdp_entry
{
    uint64_t present : 1;   // Page present in memory
    uint64_t rw : 1;   // Read-only if clear, readwrite if set
    uint64_t user : 1;   // Supervisor level only if clear
    uint64_t pwt : 1;
    uint64_t pcd : 1;
    uint64_t accessed : 1;   // Has the page been accessed since last refresh?
    uint64_t ignored : 1;
    uint64_t zero : 1;
    uint64_t mbz : 1;
    uint64_t avl : 3;
    uint64_t directory : 20;
    uint64_t reserved : 11;
    uint64_t nx : 1;
} __attribute__((packed));

typedef struct pdp_entry pdp_entry;

struct pd_entry
{
    uint64_t present : 1;   // Page present in memory
    uint64_t rw : 1;   // Read-only if clear, readwrite if set
    uint64_t user : 1;   // Supervisor level only if clear
    uint64_t pwt : 1;
    uint64_t pcd : 1;
    uint64_t accessed : 1;   // Has the page been accessed since last refresh?
    uint64_t ignored : 1;
    uint64_t zero : 1;
    uint64_t ignored2 : 1;
    uint64_t avl : 3;
    uint64_t table : 20;
    uint64_t reserved : 11;
    uint64_t nx : 1;
} __attribute__((packed));

typedef struct pd_entry pd_entry;

struct page_entry
{
    uint64_t present : 1;   // Page present in memory
    uint64_t rw : 1;   // Read-only if clear, readwrite if set
    uint64_t user : 1;   // Supervisor level only if clear
    uint64_t pwt : 1;
    uint64_t pcd : 1;
    uint64_t accessed : 1;   // Has the page been accessed since last refresh?
    uint64_t dirty : 1;   // Has the page been written to since last refresh?
    uint64_t size : 1;
    uint64_t global : 1;
    uint64_t avl : 3;
    uint64_t frame : 40;  // Frame address (shifted right 12 bits)
    uint64_t reserved : 11;
    uint64_t nx : 1;
} __attribute__((packed));

typedef struct page_entry page_entry;

typedef struct
{
    pml4_entry entry[512];
} pml_table;

typedef struct
{
    pdp_entry entry[512];
} pdp_table;

typedef struct
{   
    pd_entry entry[512];
} pd_table;

typedef struct
{
    page_entry entry[512];
} page_table;

#endif