/*

	memman.c
	
	Memory manager

	Copyright (C) 2010

	This is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2, or (at your option)
	any later version.

*/

#include <bootinfo.h>
#include <ctype.h>

#include "memman.h"
#include "isr.h"
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
    char* strMemoryType[] = { "Unknown", "Available", "Reserved", "ACPI Reclaim", "ACPI NVS Memory", "Bad stuff" };
    
    memory_region *memory_map = bootinfo->m_mmap_addr;
    uint64_t i;
    uint64_t total = 0, limit = 0;
    
    puts("Initialising memory manager... \n");

	register_handler(0x0E, page_fault_handler);
	
	puts("Analyzing memory map:\n");

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
    
	// reset limit -> limit memory to 32mb
	limit = 33488896;

    puts("Highest address of available memory: ");
    putint(limit);
    puts("\n");

    _mem_memory_size = total;
    _mem_max_blocks = limit / MEM_BLOCK_SIZE;
    _mem_used_blocks = _mem_max_blocks;
    
    // _mem_memory_map = MEM_BITMAP;
	// next block from end of kernel
	_mem_memory_map = (((uint64_t) &_kernel_end / 0x1000) + 1) * 0x1000;
	
    // this loop marks all blocks as used (0xf means all bits = 1)
    for (i = 0; i <= _mem_max_blocks / MEM_MAP_WORD_SIZE; i++)
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

    // this loop will mark first 1MB as used
    for (i = 0; i < 256; i++)
    {
        if (!mmap_test(i)) // some mem can already be reserved
        {
            _mem_used_blocks++;
            mmap_set(i);
        }
    }
	
	uint64_t blocks_used_by_kernel = 	(((uint64_t) &_kernel_end / 0x1000) + 1)
										+ ((_mem_max_blocks / MEM_BLOCKS_PER_BYTE) / MEM_BLOCK_SIZE) + 1
										- ((uint64_t) &_kernel_start / 0x1000);
	
	// this loop will mark memory used by kernel
	for (i = 0x200; i < blocks_used_by_kernel + 0x200; i++)
    {
        if (!mmap_test(i)) // some mem can already be reserved
        {
            _mem_used_blocks++;
            mmap_set(i);
        }
    }

	_kernel_next_block = (0x00000000c0000000 / 0x1000) + blocks_used_by_kernel;
	
    puts("Initialised ");
    putint(_mem_max_blocks);
    puts(" memory blocks... (");
    putint(_mem_max_blocks - _mem_used_blocks);
    puts(" blocks or ");
    putint((_mem_max_blocks - _mem_used_blocks) * MEM_BLOCK_SIZE);    
    puts(" bytes usable)\n");
    
    debug_memmap();
    
    puts("Current PML4 table: ");
    puthex(get_current_pml4());
    puts("\n");
	
	pml4_entry* pml4 = get_current_pml4();
	pml4_entry* pml4_new = mem_alloc_block();
	
	pml4[511].directory_pointer = (uint64_t) pml4_new / 0x1000;
	pml4[511].present = 1;
	pml4[511].rw = 1;
	pml4[511].user = 0;
	
	pml4_new[511].directory_pointer = (uint64_t) pml4_new / 0x1000;
	pml4_new[511].present = 1;
	pml4_new[511].rw = 1;
	pml4_new[511].user = 0;
	
	page_entry* page;
	
	for (i = 0; i < blocks_used_by_kernel; i++)
	{
		page = create_page_for_current((addr) ((void *) 0x00000000c0000000 + i * MEM_BLOCK_SIZE), 0);
		page->frame = (uint64_t) (0x200000 + i * MEM_BLOCK_SIZE) / 0x1000;
		page->present = 1;
		page->user = 0;
		page->rw = 1;
	}
	
	for (i = 0; i < 256; i++)
	{
		page = create_page_for_current((addr) ((void *) 0x0000000000000000 + i * MEM_BLOCK_SIZE), 0);
		page->frame = (uint64_t) (0x0 + i * MEM_BLOCK_SIZE) / 0x1000;
		page->present = 1;
		page->user = 0;
		page->rw = 1;
	}
	
	// creates 512 pages at virtual 3gb zone and maps to 0x100000
	//brute_create_page(0x200000, 0x00000000c0000000, blocks_used_by_kernel, pml4, 0);
    
    // creates 512 pages at virtual 0 and maps to 0
    //brute_create_page(0, 0, 256, pml4, 0);
	
	asm volatile ("xchg %bx, %bx");
	
    switch_paging((uint64_t) pml4_new);
    
    puts("Current PML4 table: ");
    puthex(get_current_pml4());
    puts("\n");

	//	(addr) ((void *) _kernel_next_block)

	
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

void invalidate()
{
	uint64_t addr;
	asm volatile("mov %%cr3, %0" : "=r" (addr));
	asm volatile("mov %0, %%cr3" : : "r" (addr));
}

page_entry* get_page(uint64_t address, pml4_entry* pml4)
{
    addr* addr = &address;
    
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

// pdp_entry* get_pdp(uint64_t address, pml4_entry* pml4)
// {
// 	return (pdp_entry*) (pml4[((addr*) &address)->pml4].directory_pointer * 0x1000);
// }

// pdp_entry* create_pdp(uint64_t address, pml4_entry* pml4, int user)
// {
// 	addr* addr = &address;
// 	pdp_entry* pdp;
// 	
// 	if (!(pml4)) {
//         return 0;
//     }
// 	
// 	if (!(pdp = pml4[addr->pml4].directory_pointer * 0x1000))
//     {
//         if (!(pdp = mem_alloc_block())) return 0;
//         pml4[addr->pml4].directory_pointer = (uint64_t) pdp / 0x1000;
//         pml4[addr->pml4].present = 1;
//         pml4[addr->pml4].rw = 1;
//         pml4[addr->pml4].user = user;
//     }
// 
// 	return pdp;
// }

page_entry* create_page_for_current(address a, int user)
{
	// uint32_t pml4 = ((addr >> 39) & 0x1FF);
	// uint32_t pdpe = ((addr >> 30) & 0x1FF);
	// uint32_t pde = ((addr >> 21) & 0x1FF);
	// uint32_t pte = ((addr >> 12) & 0x1FF);
	
	// debug_address(a);
	if (_current_pml4[a.pml4].present == 0)
	{
		// puts("kuriam pdp\n");
		_current_pml4[a.pml4].directory_pointer = (uint64_t) mem_alloc_block() / 0x1000;
		_current_pml4[a.pml4].present = 1;
		_current_pml4[a.pml4].rw = 1;
		_current_pml4[a.pml4].user = user;
		// invalidate_single(&_current_pml4[a.pml4]);
	}
	
	if (_current_pdp[a.pml4 * 512 + a.pdp].present == 0)
	{
		// puts("kuriam pd\n");
		_current_pdp[a.pml4 * 512 + a.pdp].directory = (uint64_t) mem_alloc_block() / 0x1000;
		_current_pdp[a.pml4 * 512 + a.pdp].present = 1;
		_current_pdp[a.pml4 * 512 + a.pdp].rw = 1;
		_current_pdp[a.pml4 * 512 + a.pdp].user = user;
		// invalidate_single(&_current_pdp[a.pml4 * a.pdp]);
	}
	
	if (_current_pd[(a.pml4 * 512 + a.pdp) * 512 + a.pd].present == 0)
	{
		// puts("kuriam pt\n");
		_current_pd[(a.pml4 * 512 + a.pdp) * 512 + a.pd].table = (uint64_t) mem_alloc_block() / 0x1000;
		_current_pd[(a.pml4 * 512 + a.pdp) * 512 + a.pd].present = 1;
		_current_pd[(a.pml4 * 512 + a.pdp) * 512 + a.pd].rw = 1;
		_current_pd[(a.pml4 * 512 + a.pdp) * 512 + a.pd].user = user;
		// invalidate_single(&_current_pd[a.pml4 * a.pdp * a.pd]);
	}
	
	return &_current_pt[a.frame];
}
//
// page_entry* create_page(uint64_t address, pml4_entry* pml4, int user)
// {
//     addr* addr = &address;
//     
//     pdp_entry* pdp;
//     pd_entry* pd;
//     page_entry* pt;
//     
//     if (!(pml4)) {
//         return 0;
//     }
//     
//     if (!(pdp = pml4[addr->pml4].directory_pointer * 0x1000))
//     {
//         if (!(pdp = mem_alloc_block())) return 0;
//         pml4[addr->pml4].directory_pointer = (uint64_t) pdp / 0x1000;
//         pml4[addr->pml4].present = 1;
//         pml4[addr->pml4].rw = 1;
//         pml4[addr->pml4].user = user;
//     }
// 
// 	// if (!(pdp = create_pdp(address, pml4, user)))
// 	// {
// 	// 	return 0;
// 	// }
// 	
//     if (!(pd = pdp[addr->pdp].directory * 0x1000))
//     {
//         if (!(pd = mem_alloc_block())) return 0;
//         pdp[addr->pdp].directory = (uint64_t) pd / 0x1000;
//         pdp[addr->pdp].present = 1;
//         pdp[addr->pdp].rw = 1;
//         pdp[addr->pdp].user = user;
//     }
//     
//     if (!(pt = pd[addr->pd].table * 0x1000))
//     {
//         if (!(pt = mem_alloc_block())) return 0;
//         pd[addr->pd].table = (uint64_t) pt / 0x1000;
//         pd[addr->pd].present = 1;
//         pd[addr->pd].rw = 1;
//         pd[addr->pd].user = 0;
//     }
// 
//     pt[addr->pt].present = 1;
//     pt[addr->pt].rw = 1;
//     pt[addr->pt].user = user;
// 
//     return &pt[addr->pt];
// }

//void* alloc_table(pml4_entry* pml4)
//{
//	void* physical_address = mem_alloc_block();
//	uint64_t i;
//	for (i = 0; i < 512; i++)
//	{
//		((uint64_t *) physical_address)[i] = 0;
//	}
//	create_page(physical_address + PHYS_PAGE_TABLE_PREFIX, pml4, 0);
//	return physical_address + PHYS_PAGE_TABLE_PREFIX;
//}

page_entry* create_page(uint64_t address, pml4_entry* pml4, int user)
{
    addr* addr = &address;
    
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

void* temp_map_page(void* physical_address)
{
	
}

void* alloc_kernel_page(int size)
{
	int i;
	puts("palloc() called...\n");
	
	for (i = 0; i < size; i++)
	{
		puts("Will give block ");
		puthex(_kernel_next_block);
		// page_entry* page = create_page((_kernel_next_block + 1 * i) * 0x1000, get_current_pml4(), 0);
		page_entry* page = create_page_for_current( (addr) ((void* )((_kernel_next_block + 1 * i) * 0x1000)),0);
		// if (!page) return 0;
		page->present = 1;
		page->user = 0;
		page->rw = 1;
		
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
        // page = create_page(virtual_addr + i * MEM_BLOCK_SIZE, pml4, user);
		page = create_page_for_current( (addr) ((void* )(virtual_addr + i * MEM_BLOCK_SIZE)),0);
		// if (!page) return 0;
		page->present = 1;
		page->user = 0;
		page->rw = 1;


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

void page_fault_handler(registers_t regs)
{
    // A page fault has occurred.
    // The faulting address is stored in the CR2 register.
    uint64_t faulting_address;
    asm volatile ("mov %%cr2, %0" : "=r" (faulting_address));
	
	puts("Page fault (");
	if (!(regs.err_code & 0x1)) puts("not present ");	// if page not present
	if (regs.err_code & 0x2) puts("read-only ");		// only read
    if (regs.err_code & 0x4) puts("user-mode ");		// from user space?
    if (regs.err_code & 0x8) puts("reserved ");			// overwritten CPU-reserved bits of page entry?

	puts(")! At ");
	puthex(faulting_address);
	puts("\n");

    //int id = regs.err_code & 0x10;          // Caused by an instruction fetch?
    
	for (;;);
}

/*

    Debug

*/

void debug_memmap()
{
    uint64_t i, used = 0;
    for (i = 0; i < _mem_max_blocks; i++)
    {
        if (mmap_test(i)) used = used + 1;
    }
    
    puts("Debugging ");
    putint(_mem_max_blocks);
    puts(" memory blocks: ");
    putint(_mem_max_blocks - used);
    puts(" (");
    putint((_mem_max_blocks - used) * MEM_BLOCK_SIZE);
    puts("B) free\n");
}

void debug_address(addr a)
{
	puts("Debug ");
	puthex(a);
	puts(":\n");
	puts("PML4: ");
	puthex(a.pml4);
	puts(" PDP: ");
	puthex(a.pdp);
	puts(" PD: ");
	puthex(a.pd);
	puts(" PT: ");
	puthex(a.pt);
	puts(" Offset: ");
	puthex(a.offset);
	puts("\n");
	puts("Physical frame: ");
	puthex(a.frame);
	puts("\n");
}