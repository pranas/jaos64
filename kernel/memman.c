/*

	memman.c
	
	Memory manager

	Copyright (C) 2010

	This is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2, or (at your option)
	any later version.

*/

#include "memman.h"
#include <bootinfo.h>
#include <ctype.h>


inline void mmap_set (uint64_t bit)
{
    _mem_memory_map[bit / 64] |= (1 << (bit % 64));
}

inline void mmap_unset (uint64_t bit)
{
    _mem_memory_map[bit / 64] &= ~ (1 << (bit % 64));
}

inline int mmap_test (uint64_t bit)
{
    return _mem_memory_map[bit / 64] & (1 << (bit % 64));
}

void memman_init(multiboot_info* bootinfo)
{
    char* strMemoryType[] = { "Unknown", "Available", "Reserved", "ACPI Reclaim", "ACPI NVS Memory", "Bad" };
    
    memory_region *memory_map = bootinfo->m_mmap_addr;
    uint64_t i;
    uint64_t total = 0, limit = 0;
    
    puts("Initialising memory manager...\n");
    puts("Analyzing memoy map:\n");
    for (i = 0; i < bootinfo->m_mmap_length; i++)
    {
        if (memory_map[i].type == 1)
        {
            if ((memory_map[i].start + memory_map[i].size) > limit) { limit = memory_map[i].start + memory_map[i].size; }
            total = total + memory_map[i].size;
        }
        puts("   From: ");
        puthex(memory_map[i].start);
        puts("    Size: ");
        puthex(memory_map[i].size);
        puts("    Type: ");
        puts(strMemoryType[memory_map[i].type]);
        puts("\n");
    }    
    puts("\nFound ");
    putint(total);
    puts(" bytes of usable memory...\n");
    
    puts("Highest address of available memory: ");
    putint(limit);
    puts("\n");
    
    //_mem_memory_size = total;
    
    _mem_max_blocks = limit / MEM_BLOCK_SIZE;
    _mem_used_blocks = _mem_max_blocks;
    
    _mem_memory_map = 0x20000;
    
    for (i = 0; i <= (_mem_max_blocks / MEM_BLOCKS_PER_BYTE)/8; i++)
    {
        _mem_memory_map[i] = 0-1; //0xffffffffffffffff;
    }
    
    puts("Memory map put at ");
    puthex(_mem_memory_map);
    puts(" and it's taking ");
    putint(_mem_max_blocks / MEM_BLOCKS_PER_BYTE);
    puts(" bytes...\n");
    
    for (i = 0; i < bootinfo->m_mmap_length; i++)
    {
        if (memory_map[i].type == 1)
        {
            mem_init_region(memory_map[i].start, memory_map[i].size);
        }
    }
        
    // our boot loader mapped 3mb of physical mem
    // 0x -> 0x (2MB)
    // 0x3gb -> 0x1mb (2MB)
    // total 3MB (cause those ranges overlap)
    // 3MB = 767 blocks
    for (i = 0; i < 767; i++)
    {
        // if (!mmap_test(i)) _mem_used_blocks++; // some mem can already be reserved
        // mmap_set(i);
    }
    
    
    puts("Initialised ");
    putint(_mem_max_blocks);
    puts(" memory blocks... (");
    putint(_mem_max_blocks - _mem_used_blocks);
    puts(" blocks or ");
    putint((_mem_max_blocks - _mem_used_blocks) * MEM_BLOCK_SIZE);    
    puts(" bytes usable)\n");
    
    debug_memmap(_mem_max_blocks);
    
    asm volatile ("xchg %bx, %bx");
}

void mem_init_region(uint64_t base, uint64_t size)
{
    uint64_t align = base / MEM_BLOCK_SIZE ;
	uint64_t blocks = size / MEM_BLOCK_SIZE;
    
    for (; blocks>0; blocks--) {
        mmap_unset(align++);
        _mem_used_blocks--;
    }
}

void debug_memmap(uint64_t blocks)
{
    uint64_t i, used = 0;
    for (i = 0; i < blocks; i++)
    {
        if (mmap_test(i)) used = used + 1;
    }
    
    puts("Debugging ");
    putint(blocks);
    puts(" memory blocks: ");
    putint(blocks - used);
    puts(" (");
    putint((blocks - used) * MEM_BLOCK_SIZE);
    puts("B) free\n");
}