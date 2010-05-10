#pragma once
#ifndef MEMMAN_H
#define MEMMAN_H

/*

	memman.h

	Memory manager

	Copyright (C) 2010

	This is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2, or (at your option)
	any later version.

*/

#include "isr.h"
#include <stdint.h>
#include <bootinfo.h>
#include <ctype.h>
#include "kheap.h"

#define MEM_BLOCKS_PER_BYTE 8
#define MEM_BYTES_PER_WORD 8
#define MEM_MAP_WORD_SIZE 64
#define MEM_BLOCK_SIZE	4096
#define MEM_BLOCK_ALIGN	MEM_BLOCK_SIZE
//#define MEM_BITMAP 0x00000000c0200000 //0x20000
// #define PHYS_PAGE_TABLE_PREFIX 0x0000000100000000
#define MEM_DEBUG 1

#define invalidate_single(addr) asm volatile ("invlpg %0": :"m" (*(char *) addr));

// http://forum.osdev.org/viewtopic.php?f=1&t=20719&start=0
// http://forum.osdev.org/viewtopic.php?f=1&t=21193
// http://forum.osdev.org/viewtopic.php?f=15&t=18379

#define CURRENT_PML4_PREFIX 0xFFFFFFFFFFFFF000
#define CURRENT_PDP_PREFIX 0xFFFFFFFFFFE00000
#define CURRENT_PD_PREFIX 0xFFFFFFFFC000000
#define CURRENT_PT_PREFIX 0xFFFFFF8000000000

extern uint64_t _kernel_start;
extern uint64_t _kernel_end;

// format of entry in BIOS memory map
struct memory_region
{
    uint64_t	start;
    uint64_t	size;
	uint32_t	type;
	uint32_t	acpi3;
} __attribute__((packed));

typedef struct memory_region memory_region;

union address {
	uint64_t hex;
	void* ptr;
	struct {
	    uint64_t offset: 12;
	    uint64_t pt: 9;
	    uint64_t pd: 9;
	    uint64_t pdp: 9;
	    uint64_t pml4: 9;
	    uint64_t sign: 16;		
	};
	struct {
	    uint64_t : 12;
	    uint64_t frame: 36;
	    uint64_t : 16;		
	};
};

typedef union address addr;
typedef union address address;

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
    uint64_t directory_pointer : 40;
    uint64_t reserved : 11;
    uint64_t nx : 1;
} __attribute__((packed));

typedef struct pml4_entry pml4_entry;

struct pdp_entry
{
    uint64_t present : 1;
    uint64_t rw : 1;
    uint64_t user : 1;
    uint64_t pwt : 1;
    uint64_t pcd : 1;
    uint64_t accessed : 1;
    uint64_t ignored : 1;
    uint64_t zero : 1;
    uint64_t mbz : 1;
    uint64_t avl : 3;
    uint64_t directory : 40;
    uint64_t reserved : 11;
    uint64_t nx : 1;
} __attribute__((packed));

typedef struct pdp_entry pdp_entry;

struct pd_entry
{
    uint64_t present : 1;
    uint64_t rw : 1;
    uint64_t user : 1;
    uint64_t pwt : 1;
    uint64_t pcd : 1;
    uint64_t accessed : 1;
    uint64_t ignored : 1;
    uint64_t zero : 1;
    uint64_t ignored2 : 1;
    uint64_t avl : 3;
    uint64_t table : 40;
    uint64_t reserved : 11;
    uint64_t nx : 1;
} __attribute__((packed));

typedef struct pd_entry pd_entry;

struct page_entry
{
    uint64_t present : 1;		// Present (P) bit, when 0 all other ignored (can be user by software)
    uint64_t rw : 1;			// Read/Write (R/W) bit, RO if 0, RW if 1
    uint64_t user : 1;   		// User/Supervisor (U/S) bit, S if 0, U if 1
    uint64_t pwt : 1;			// Page-Level Writethrough (PWT) bit
    uint64_t pcd : 1;			// Page-Level Cache Disable (PCD) bit
    uint64_t accessed : 1;		// Accessed (A) bit, set by CPU when read or written, never cleared by CPU
    uint64_t dirty : 1;			// Dirty (D) bit, set by CPU when written, never cleared by CPU
    uint64_t size : 1;			// Page Size (PS) bit, if set it's lowest in page hierarchy
    uint64_t global : 1;		// Global Page (G) bit, indicates global pages which are not invalidated when switching, to use needs CR4.PGE=1
    uint64_t avl : 3;			// Available for software
    uint64_t frame : 40;  		// Physical frame address (shifted right 12 bits)
    uint64_t reserved : 11;		// Reserved bit should always be cleared or #PF will occur if LM or PAE enabled
	uint64_t nx : 1;			// No execution bit
} __attribute__((packed));

typedef struct page_entry page_entry;

// private functions (should not be used outside memory manager)
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

pml4_entry* clone_pml4t();

// replaces the same pml4
void invalidate();
void* alloc_page(void* virtual, uint64_t size);
uint64_t get_current_pml4();
void switch_paging(void*);
int brute_create_page(uint64_t physical_addr, uint64_t virtual_addr, uint64_t size, pml4_entry* pml4, int user);
page_entry* get_page(uint64_t physical_address, pml4_entry* pml4);
pdp_entry* get_pdp(uint64_t address, pml4_entry* pml4);
page_entry* create_page(uint64_t address, pml4_entry* pml4, int user);
page_entry* create_page_for_current(address a, int user);
pdp_entry* create_pdp(uint64_t address, pml4_entry* pml4, int user);
void* alloc_kernel_page(int size);
void* alloc_table(pml4_entry* pml4);
void copy_page_tables(pml4_entry* from, pml4_entry* to);
void free_kernel_page(void* address, uint64_t size);
void page_fault_handler(registers_t* regs);

// debug procedures
void debug_memmap();
void debug_address(address);

#endif
