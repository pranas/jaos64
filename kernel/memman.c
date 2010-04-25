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

/*

    Private functions used by memory manager

*/

inline void mmap_set (uint64_t bit)
{
    _mem_memory_map[bit / 64] |= ((uint64_t) 1 << (bit % 64));
}

inline void mmap_unset (uint64_t bit)
{
    _mem_memory_map[bit / 64] &= ~ ((uint64_t) 1 << (bit % 64));
}

inline uint64_t mmap_test (uint64_t bit)
{
    return _mem_memory_map[bit / 64] & ((uint64_t) 1 << (bit % 64));
}

uint64_t mmap_first_free()
{
    uint64_t i, j;
    for (i = 0; i < _mem_max_blocks / 64; i++)
    {
        if (_mem_memory_map[i] != 0-1)
        {
            for (j = 0; j < 64; j++)
            {
                if(!(_mem_memory_map[i] & ((uint64_t) 1 << j)))
                {
                    return i*64+j;
                }
            }
        }
    }
    return 0;
}

uint64_t mmap_first_free_zone(uint64_t size)
{
    if (size == 0)
		return 0;

	if (size == 1)
		return mmap_first_free();
		
    uint64_t i, j, l;
    
    for (i = 0; i < _mem_max_blocks / 64; i++)
    {
        if (_mem_memory_map[i] != 0-1)
        {
            for (j = 0; j < 64; j++)
            {
                if(!(_mem_memory_map[i] & ((uint64_t) 1 << j)))
                {
					uint64_t count;
					for (count=0; count <= size; count++) {
						if (count == size)
							return i*64+j;

						if ( mmap_test (i*64+j+count) )
                            break;
					}
					// if we would stop here next loop whould check the same space only 1 block
					// smaller
					// we need to jump over this block cause we know that it's too small				
					
                    // puts("Started our search from ");
                    // putint(i*64+j);
                    // puts(" block, but at ");
                    // putint(i*64+j+count);
                    // puts(" found used block..\n");
                    // puts("Jumping i=");
					i = (i*64+j+count) / 64;
                    // putint(i);
                    // puts("j = ");
                    j = (i*64+j+count) % 64;
                    // putint(j);
                    // puts("\n");
                }
            }
        }
    }
    return 0;
}

/*

    Public interface for memory manager

*/

void memman_init(multiboot_info* bootinfo)
{
    char* strMemoryType[] = { "Unknown", "Available", "Reserved", "ACPI Reclaim", "ACPI NVS Memory", "Bad" };
    
    memory_region *memory_map = bootinfo->m_mmap_addr;
    uint64_t i;
    uint64_t total = 0, limit = 0;
    
    puts("Initialising memory manager...\n");
    puts("Analyzing memory map:\n");
    
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
    
    _mem_memory_size = total;
    _mem_max_blocks = limit / MEM_BLOCK_SIZE;
    _mem_used_blocks = _mem_max_blocks;
    
    _mem_memory_map = MEM_BITMAP;
    
    // this loop marks all blocks as used (0xf means all bits = 1)
    for (i = 0; i <= (_mem_max_blocks / MEM_BLOCKS_PER_BYTE) / MEM_BYTES_PER_WORD; i++)
    {
        _mem_memory_map[i] = 0-1; // 0-1 = 0xf..f;
    }
    
    puts("Memory map put at ");
    puthex(_mem_memory_map);
    puts(" and it's taking ");
    putint(_mem_max_blocks / MEM_BLOCKS_PER_BYTE);
    puts(" bytes...\n");
    
    // finds entries of usable memory in memory map and initialises
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
    // this loop will mark them as used
    for (i = 0; i < 767; i++)
    {
        if (!mmap_test(i)) // some mem can already be reserved
        {
            _mem_used_blocks++;
            mmap_set(i);
        }
    }
    
    puts("Initialised ");
    putint(_mem_max_blocks);
    puts(" memory blocks... (");
    putint(_mem_max_blocks - _mem_used_blocks);
    puts(" blocks or ");
    putint((_mem_max_blocks - _mem_used_blocks) * MEM_BLOCK_SIZE);    
    puts(" bytes usable)\n");
    
    debug_memmap(_mem_max_blocks);
    
    // mmap_unset(3);
    // mmap_unset(124);
    // mmap_unset(125);
    // mmap_unset(126);
    // 
    // for (i = 200; i < 300; i++) mmap_unset(i);
    // 
    // puts("First zone for 3 blocks: \n");
    // putint(mmap_first_free_zone(10));
    // puts("\n");
    
    
    
    asm volatile ("xchg %bx, %bx");
}

void mem_init_region(uint64_t base, uint64_t size)
{
    // TODO: for faster init make use of uint64_t (mark all 64 blocks at once)
    
    uint64_t align = base / MEM_BLOCK_SIZE ;
	uint64_t blocks = size / MEM_BLOCK_SIZE;
    
    for (; blocks>0; blocks--) {
        if (mmap_test(align))
        {
            // conditional helps when trying to initiate same region (or overlapping region)
            mmap_unset(align++);
            _mem_used_blocks--;
        }
    }
}


void* mem_alloc_block()
{
    if (mem_free_block_count() == 0)
        return 0;
        
    uint64_t frame = mmap_first_free();
    
    if (frame == 0)
        return 0;
        
    mmap_set(frame);
    _mem_used_blocks++;
    
    return (void*) ((uint64_t) frame * MEM_BLOCK_SIZE);
}

void mem_free_block(void* addr)
{
    mmap_unset((uint64_t) addr / MEM_BLOCK_SIZE);
    _mem_used_blocks--;
}

void* mem_alloc_blocks(uint64_t size)
{
    if (mem_free_block_count() < size)
        return 0;
        
    uint64_t frame = mmap_first_free_zone(size);
    
    if (frame == 0)
        return 0;
    
    uint64_t i;    
    
    for (i = 0; i < size; i++)
		mmap_set(frame+i);
	
	_mem_used_blocks += size;
	
	return (void*) ((uint64_t) frame * MEM_BLOCK_SIZE);
}

void mem_free_blocks(void* addr, uint64_t size)
{
    uint64_t i;
    uint64_t frame = (uint64_t) addr / MEM_BLOCK_SIZE;
    for (i = frame; i < frame+size; i++)
        mmap_unset(i);
        
    _mem_used_blocks -= size;
}

uint64_t mem_free_block_count()
{
    return _mem_max_blocks - _mem_used_blocks;
}


/*

    Debug

*/

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