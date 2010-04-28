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
#include "isr.h"
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
                    uint64_t i2 = (i*64+j+count) / 64;
                    uint64_t j2 = (i*64+j+count) % 64;
					i = i2;
                    j = j2;
                    // putint(i);
                    // puts("j = ");
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
    
    puts("Initialising memory manager... "); puts("Analyzing memory map:\n");
    for (i = 0; i < bootinfo->m_mmap_length; i++)
    {
        if (memory_map[i].type == 1)
        {
            if ((memory_map[i].start + memory_map[i].size) > limit) { limit = memory_map[i].start + memory_map[i].size; }
            total = total + memory_map[i].size;
        }
        puts("   From: "); puthex(memory_map[i].start);
        puts("    Size: ");
        puthex(memory_map[i].size);
        puts("    Type: ");
        puts(strMemoryType[memory_map[i].type]);
        puts("\n");
    }
    
    puts("Found ");
    putint(total);
    puts(" bytes of usable memory...\n");
    
    puts("Highest address of available memory: ");
    putint(limit);
    puts("\n");

	// reset limit -> limit memory to 32mb
	limit = 33091584;
    
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
    // this loop will mark 2MB as used (1MB left for kernel neads)
    for (i = 0; i < 511; i++)
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
    
	// extend current map
	brute_create_page(0, 0, 1024, get_current_pml4(), 0);

    puts("Current PML4 table: ");
    puthex(get_current_pml4());
    puts("\n");
    
    pml4_entry* pml4 = mem_alloc_block();

    // creates 512 pages at virtual 3gb zone and maps to 0x100000
    brute_create_page(0x100000, 0x00000000c0000000, 512, pml4, 0);

	// probably 512 is enough, but let's say 513
	_kernel_next_block = (0x00000000c0000000 / 0x1000) + 512 ;
    
    // creates 512 pages at virtual 0 and maps to 0
    brute_create_page(0, 0, 1024, pml4, 0);
    
    switch_paging((uint64_t) pml4);
    
    puts("Current PML4 table: ");
    puthex(get_current_pml4());
    puts("\n");
    
	// asm volatile ("xchg %bx, %bx");
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

void switch_paging(void* addr)
{
    asm volatile("mov %0, %%cr3":: "r"(addr));
}

uint64_t get_current_pml4()
{
    uint64_t addr;
    asm volatile ("mov %%cr3, %0" : "=a"(addr));
    return addr;
}

page_entry* get_page(uint64_t address, pml4_entry* pml4)
{
    virtual_addr* addr = &address;
    
    pdp_entry* pdp;
    pd_entry* pd;
    page_entry* pt;
    
    if (pdp = pml4[addr->pml4].directory_pointer * 0x1000)
    {
        if (pd = pdp[addr->pdp].directory * 0x1000)
        {
            if (pt = pd[addr->pd].table * 0x1000)
            {
                if (pt[addr->pt].frame) return &pt[addr->pt];
            }
        }
    }
    return 0;
}

page_entry* create_page(uint64_t address, pml4_entry* pml4, int user)
{
    virtual_addr* addr = &address;
    
    pdp_entry* pdp;
    pd_entry* pd;
    page_entry* pt;
    
    if (!(pml4)) {
        return 0;
    }
    
    if (!(pdp = pml4[addr->pml4].directory_pointer * 0x1000))
    {
        if (!(pdp = mem_alloc_block())) return 0;
        pml4[addr->pml4].directory_pointer = (uint64_t) pdp / 0x1000;
        pml4[addr->pml4].present = 1;
        pml4[addr->pml4].rw = 1;
        pml4[addr->pml4].user = user;
    }
    
    if (!(pd = pdp[addr->pdp].directory * 0x1000))
    {
        if (!(pd = mem_alloc_block())) return 0;
        pdp[addr->pdp].directory = (uint64_t) pd / 0x1000;
        pdp[addr->pdp].present = 1;
        pdp[addr->pdp].rw = 1;
        pdp[addr->pdp].user = user;
    }
    
    if (!(pt = pd[addr->pd].table * 0x1000))
    {
        if (!(pt = mem_alloc_block())) return 0;
        pd[addr->pd].table = (uint64_t) pt / 0x1000;
        pd[addr->pd].present = 1;
        pd[addr->pd].rw = 1;
        pd[addr->pd].user = 0;
    }
    pt[addr->pt].present = 1;
    pt[addr->pt].rw = 1;
    pt[addr->pt].user = user;
    return &pt[addr->pt];
}

void* alloc_kernel_page(int size)
{
	int i;
	puts("palloc() called...\n");
	
	for (i = 0; i < size; i++)
	{
		puts("Will give block ");
		puthex(_kernel_next_block);
		page_entry* page = create_page((_kernel_next_block + 1 * i) * 0x1000, get_current_pml4(), 0);
		// if (!page) return 0;
	
		void* phys_frame = mem_alloc_block();
		// if (!phys_frame) return 0;
	
		puts(" put at ");
		puthex((uint64_t) phys_frame);
		puts(" physical memory...\n");
	
		page->frame = (uint64_t) phys_frame / 0x1000;
	}
	_kernel_next_block += size;
	return (void*) ((uint64_t) (_kernel_next_block - size) * 0x1000);
}

void free_kernel_page(void* address)
{
	
}

int brute_create_page(uint64_t physical_addr, uint64_t virtual_addr, uint64_t size, pml4_entry* pml4, int user)
{
    uint64_t i;
    page_entry* page;
    
    // approximation how many blocks we will need, it's actually more if size > 512*512 (this counts only page_tables, not pd, pdp, pml4)
    if (size / 512 > mem_free_block_count()) return 0;
    
    for (i = 0; i < size; i++)
    {
        page = create_page(virtual_addr + i * MEM_BLOCK_SIZE, pml4, user);
        if (!page)
        {
            // should revert changes to free memory and report error
            return i+1;
            // but we will return only count of pages made and that's all :D
        }
        page->frame = (physical_addr / 0x1000) + i;
    }
    
    return i;
}

void page_fault(registers_t regs)
{
    // A page fault has occurred.
    // The faulting address is stored in the CR2 register.
    uint64_t faulting_address;
    asm volatile ("mov %%cr2, %0" : "=r" (faulting_address));
	puts("Page fault! ");
	puthex(faulting_address);
	puts("\n");
    // 
    // // The error code gives us details of what happened.
    // int present   = !(regs.err_code & 0x1); // Page not present
    // int rw = regs.err_code & 0x2;           // Write operation?
    // int us = regs.err_code & 0x4;           // Processor was in user-mode?
    // int reserved = regs.err_code & 0x8;     // Overwritten CPU-reserved bits of page entry?
    // int id = regs.err_code & 0x10;          // Caused by an instruction fetch?
    // 
    // // Output an error message.
    // monitor_write("Page fault! ( ");
    // if (present) {monitor_write("present ");}
    // if (rw) {monitor_write("read-only ");}
    // if (us) {monitor_write("user-mode ");}
    // if (reserved) {monitor_write("reserved ");}
    // monitor_write(") at 0x");
    // monitor_write_hex(faulting_address);
    // monitor_write("\n");
    // PANIC("Page fault");
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
