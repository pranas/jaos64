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

					uint64_t i2 = (i*64+j+count) / 64;
					uint64_t j2 = (i*64+j+count) % 64;
					i = i2;
					j = j2;
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

	puts("Initializing memory manager... \n");

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
	//limit = 33488896;

	puts("Highest address of available memory: ");
	putint(limit);
	puts("\n");

	_mem_memory_size = total;
	_mem_max_blocks = limit / MEM_BLOCK_SIZE;
	_mem_used_blocks = _mem_max_blocks;

	// we will put our memory map dynamically at next block after kernel
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

	// finds entries of usable memory in memory map and initializes
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

	// how many blocks kernel is taking?
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

	// this code will initialize paging

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

	page = create_page_for_current((addr) (void *) 0x00000000fffff000, 0);
	page->frame = 8;
	page->present = 1;
	page->user = 0;
	page->rw = 1; 

	// map video memory to kernel space
	set_video_memory(_kernel_next_block * 0x1000);
	page = create_page_for_current((addr) ((uint64_t) _kernel_next_block * 0x1000), 0);
	page->frame = 0xB8;
	page->present = 1;
	page->user = 0;
	page->rw = 1;

	_kernel_next_block++;

	// enable new paging tables
	switch_paging((uint64_t) pml4_new);

	puts("Current PML4 table: ");
	puthex(get_current_pml4());
	puts("\n");

	_kernel_pml4t = pml4_new; // not used!

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

pml4_entry get_pml4_entry(uint64_t i)
{
	return _current_pml4[i];
}

pdp_entry get_pdp_entry(uint64_t a, uint64_t b)
{
	return _current_pdp[a * 512 + b];
}

pd_entry get_pd_entry(uint64_t a, uint64_t b, uint64_t c)
{
	return _current_pd[(a * 512 + b) * 512 + c];
}

page_entry get_page_entry(uint64_t a, uint64_t b, uint64_t c, uint64_t d)
{
	return _current_pt[((a * 512 + b) * 512 + c) * 512 + d];
}

void* get_physical_address(void* a)
{
	return _current_pt[((addr) a).frame].frame * 0x1000;
}


// works only when current paging table is self referenced at end of memory (pml4[511])
page_entry* create_page_for_current(address a, int user)
{
	// uint32_t pml4 = ((addr >> 39) & 0x1FF);
	// uint32_t pdpe = ((addr >> 30) & 0x1FF);
	// uint32_t pde = ((addr >> 21) & 0x1FF);
	// uint32_t pte = ((addr >> 12) & 0x1FF);

	if (_current_pml4[a.pml4].present == 0)
	{
		_current_pml4[a.pml4].directory_pointer = (uint64_t) mem_alloc_block() / 0x1000;
		_current_pml4[a.pml4].present = 1;
		_current_pml4[a.pml4].rw = 1;
		_current_pml4[a.pml4].user = user;
		// invalidate_single(&_current_pml4[a.pml4]);
	}

	if (_current_pdp[a.pml4 * 512 + a.pdp].present == 0)
	{
		_current_pdp[a.pml4 * 512 + a.pdp].directory = (uint64_t) mem_alloc_block() / 0x1000;
		_current_pdp[a.pml4 * 512 + a.pdp].present = 1;
		_current_pdp[a.pml4 * 512 + a.pdp].rw = 1;
		_current_pdp[a.pml4 * 512 + a.pdp].user = user;
		// invalidate_single(&_current_pdp[a.pml4 * a.pdp]);
	}

	if (_current_pd[(a.pml4 * 512 + a.pdp) * 512 + a.pd].present == 0)
	{
		_current_pd[(a.pml4 * 512 + a.pdp) * 512 + a.pd].table = (uint64_t) mem_alloc_block() / 0x1000;
		_current_pd[(a.pml4 * 512 + a.pdp) * 512 + a.pd].present = 1;
		_current_pd[(a.pml4 * 512 + a.pdp) * 512 + a.pd].rw = 1;
		_current_pd[(a.pml4 * 512 + a.pdp) * 512 + a.pd].user = user;
		// invalidate_single(&_current_pd[a.pml4 * a.pdp * a.pd]);
	}

	return &_current_pt[a.frame];
}

void* temp_map_page(void* physical_address)
{

}

void* map_mem_block()
{

}

void* alloc_page(void* virtual, uint64_t size)
{
	uint64_t i;
	
	if (size == 0) size = 1;
	
	for (i = 0; i < size; i++)
	{
		void* phys_frame = mem_alloc_block();
		if (!phys_frame) return 0;
		
		page_entry* page = create_page_for_current( (addr) (virtual + i * 0x1000), 1);
		if (!page) return 0;
		
		page->present = 1;
		page->user = 1;
		page->rw = 1;

		page->frame = (uint64_t) phys_frame / 0x1000;
	}
	return virtual;
}

void* alloc_kernel_page(int size)
{
	uint64_t i;

	for (i = 0; i < size; i++)
	{
		void* phys_frame = mem_alloc_block();
		if (!phys_frame) return 0;
		
		page_entry* page = create_page_for_current( (addr) ((void* )((_kernel_next_block + 1 * i) * 0x1000)),0);
		if (!page) return 0;
		
		page->present = 1;
		page->user = 0;
		page->rw = 1;

		page->frame = (uint64_t) phys_frame / 0x1000;
	}
	_kernel_next_block += size;
	return (void*) ((uint64_t) (_kernel_next_block - size) * 0x1000);
}

void free_kernel_page(void* address)
{

}

void mem_copy_block(void* from, void* to)
{
	uint16_t i;

	// puts("Making copy of ");
	// puthex((uint64_t) from);
	// puts(" to ");
	// puthex((uint64_t) to);
	// puts("\n");

	for (i = 0; i < 512; i++)
	{
		((uint64_t *) to)[i] = ((uint64_t *) from)[i];
	}
}

// clones current paging scheme
// return physical address of new pml4
pml4_entry* clone_pml4t()
{
	pml4_entry* pml4 = alloc_kernel_page(1);
	pdp_entry* pdp = alloc_kernel_page(1);
	pd_entry* pd;
	page_entry* pt;

	uint64_t i, z, y;

	for (i = 0; i < 512; i++)
	{
		((uint64_t *)pml4)[i] = 0;
		((uint64_t *)pdp)[i] = 0;
	}

	// only pml4[0]
	pml4[0].directory_pointer = (uint64_t) get_physical_address(pdp) / 0x1000;
	pml4[0].present = 1;
	pml4[0].rw = 1;
	pml4[0].user = 1;

	// to simplify this procedure for now
	// copy pdp[0..2] and link pdp[3] (kernel)

	for (i = 0; i < 3; i++)
	{
		if (get_pdp_entry(0, i).present == 1)
		{
			pd = alloc_kernel_page(1);
			pdp[i].directory = (uint64_t) get_physical_address(pd) / 0x1000;
			pdp[i].present = 1;
			pdp[i].rw = 1;
			pdp[i].user = 1;

			for (z = 0; z < 512; z++)
			{
				if(get_pd_entry(0, i, z).present == 1)
				{
					pt = alloc_kernel_page(1);
					pd[z].table = (uint64_t) get_physical_address(pt) / 0x1000;
					pd[z].present = 1;
					pd[z].rw = 1;
					pd[z].user = 1;
					for (y = 0; y < 512; y++)
					{
						if(get_page_entry(0, i, z, y).present == 1)
						{
							void* new_page = alloc_kernel_page(1);
							address original_page = { 0 };
							original_page.pml4 = 0; 
							original_page.pdp = i;
							original_page.pd = z;
							original_page.pt = y;
							mem_copy_block(original_page.ptr, new_page);
							pt[y].frame = (uint64_t) get_physical_address(new_page) / 0x1000; //get_page_entry(0, i, z, y);
							pt[y].user = 1;
							pt[y].present = 1;
							pt[y].rw = 1;
						}
					}
				}
			}
		}
	}

	pdp[3] = get_pdp_entry(0, 3);

	// set pml4[511] to itself
	// this makes "self reference"
	// and allows to reach paging tables

	pml4[511].directory_pointer = (uint64_t) get_physical_address(pml4) / 0x1000;
	pml4[511].present = 1;
	pml4[511].rw = 1;
	pml4[511].user = 0;

	return get_physical_address(pml4);
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
			// should revert changes to free memory and report error (TODO)
			return i+1;
			// but we will return only count of pages made and that's all
		}
		page->frame = (physical_addr / 0x1000) + i;
	}

	return i;
}

void page_fault_handler(registers_t* regs)
{
	// A page fault has occurred.
	// The faulting address is stored in the CR2 register.
	uint64_t faulting_address;
	asm volatile ("mov %%cr2, %0" : "=r" (faulting_address));

	puts("Page fault (");
	if (!(regs->err_code & 0x1)) puts("not present ");	// if page not present
	if (regs->err_code & 0x2) puts("read-only ");		// only read
	if (regs->err_code & 0x4) puts("user-mode ");		// from user space?
	if (regs->err_code & 0x8) puts("reserved ");			// overwritten CPU-reserved bits of page entry?

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
