#include "kheap.h"

heap_t *kheap = 0;

void kheap_init()
{
	if (KHEAP_INITIAL_SIZE - HEAP_INDEX_SIZE * 8 + MEM_BLOCK_SIZE < HEAP_MIN_SIZE)
	{
		puts("[KHEAP] cannot initialize heap due to bad size constants.\n");
	}
	else
	{
		// Initialise the kernel heap.
		uint64_t kheap_start = (uint64_t) alloc_kernel_page(KHEAP_INITIAL_SIZE / MEM_BLOCK_SIZE);
		uint64_t kheap_end   = kheap_start + KHEAP_INITIAL_SIZE;
		kheap = create_heap(kheap_start, kheap_end, 0xd0000000, 0, 0);
		puts("[KHEAP] kernel heap initialized.\n");
	}
}
// prints handy heap information, 0 -- alloc, 1 -- free
static void heap_debug(int n)
{
	if (n == 0)
		puts("[ALLOC] ");
	else if (n == 1)
		puts("[FREE] ");
	puts("start: "); puthex(kheap->start_address);
	puts(" end: "); puthex(kheap->end_address);
	puts(" max: "); puthex(kheap->max_address);
	puts("\n");
}

void print_index()
{
	ordered_array_t arr = kheap->index;
	uint64_t i;
	for (i = 0; i < arr.size; i++)
	{
		header_t* header = (header_t*) arr.array[i];
		puts("Header "); putint(i);
		puts(" position: "); puthex((uint64_t) header);
		puts(" magic: "); puthex((uint64_t) header->magic);
		puts(" is_hole: "); putint((uint64_t) header->is_hole);
		puts(" size: "); puthex(header->size);
		puts("\n");
	}
}

uint64_t kmalloc_int(uint64_t sz, int align, uint64_t *phys)
{
//	if (align)
//	{
//		puts("aligned kmalloc");
//		puts("size: "); puthex(sz); puts("\n");
//		magicbp();
//		print_index();
//	}
	if (sz == 0)
		return 0;
	if (kheap != 0)
	{
//		puts("calling alloc\n");
		void *addr = alloc(sz, (uint8_t)align, kheap);
		/* TODO: figure out what to do with frame physical addresses
		if (phys != 0)
		{
			page_t *page = get_page((uint32_t)addr, 0, kernel_directory);
			*phys = page->frame*0x1000 + (uint32_t)addr&0xFFF;
		}
		*/
//		if (align)
//		{
//			print_index();
//			puts("address: "); puthex(addr); puts("\n");
//			puts("aligned kmalloc end\n");
//			magicbp();
//		}
		return (uint64_t) addr;
	}
	else
	{
		if (sz & 0x00000FFF)
		{
			sz &= 0xFFFFF000;
			sz /= 0x1000;
		}
		return (uint64_t) alloc_kernel_page(sz);
	}
}

void kfree(void *p)
{
	free(p, kheap);
}

uint64_t kmalloc_a(uint64_t sz)
{
	return kmalloc_int(sz, 1, 0);
}

uint64_t kmalloc_p(uint64_t sz, uint64_t *phys)
{
	return kmalloc_int(sz, 0, phys);
}

uint64_t kmalloc_ap(uint64_t sz, uint64_t *phys)
{
	return kmalloc_int(sz, 1, phys);
}

uint64_t kmalloc(uint64_t sz)
{
	return kmalloc_int(sz, 0, 0);
}

static void expand(uint64_t new_size, heap_t *heap)
{
	// Sanity check.
	puts("expanding\n");
	if (new_size < heap->end_address - heap->start_address)
		puts("expand: new_size smaller than current size\n");

	// Get the nearest following page boundary.
	if (new_size & 0x00000FFF)
	{
		new_size &= 0xFFFFF000;
		new_size += 0x1000;
	}

	// Make sure we are not overreaching ourselves.
	if (heap->start_address+new_size > heap->max_address)
		puts("expand: expanded address exceeds maximum\n");

	uint64_t old_size = heap->end_address - heap->start_address;

	alloc_kernel_page((new_size - old_size) / MEM_BLOCK_SIZE); // TODO: supervisor and readonly bits
	heap->end_address = heap->start_address+new_size;
}

static uint64_t contract(uint64_t new_size, heap_t *heap)
{
	// Sanity check.
	puts("contracting\n");
	if (new_size >= heap->end_address - heap->start_address)
		puts("contract: contracting, but new_size is larger than old_size\n");

	// Get the nearest following page boundary.
	if (new_size & 0x00000FFF)
	{
		new_size &= 0xFFFFF000;
		new_size += 0x1000;
	}

	// Don't contract too far!
	if (new_size < HEAP_MIN_SIZE)
		new_size = HEAP_MIN_SIZE;

	uint64_t old_size = heap->end_address-heap->start_address;

	free_kernel_page((void*) heap->start_address + new_size, (old_size - new_size) / MEM_BLOCK_SIZE);
	heap->end_address = heap->start_address + new_size;
	return new_size;
}

static uint64_t find_smallest_hole(uint64_t size, uint8_t page_align, heap_t *heap)
{
	// Find the smallest hole that will fit.
	uint64_t iterator = 0;
	while (iterator < heap->index.size)
	{
		header_t *header = (header_t *)lookup_ordered_array(iterator, &heap->index);
		// If the user has requested the memory be page-aligned
		if (page_align > 0)
		{
			// Page-align the starting point of this header.
			uint64_t location = (uint64_t)header;
			uint64_t offset = 0;
			if ((location+sizeof(header_t)) & 0x00000FFF)
				offset = MEM_BLOCK_SIZE - (location+sizeof(header_t)) % MEM_BLOCK_SIZE;
			uint64_t hole_size = (uint64_t) header->size - offset;
			// Can we fit now?
			if (header->size > offset && hole_size >= size)
				break;
		}
		else if (header->size >= size)
			break;
		iterator++;
	}
	// Why did the loop exit?
	if (iterator == heap->index.size)
		return -1; // We got to the end and didn't find anything.
	else
		return iterator;
}

static uint8_t header_t_less_than(void*a, void *b)
{
	return (((header_t*)a)->size < ((header_t*)b)->size)?1:0;
}

heap_t *create_heap(uint64_t start, uint64_t end_addr, uint64_t max, uint8_t supervisor, uint8_t readonly)
{
	heap_t *heap = (heap_t*) start;

	if (start & 0x00000FFF)
		puts("Start address not page-aligned\n");
	if (end_addr & 0x00000FFF)
		puts("End address not page-aligned\n");

	// Initialise the index.
	// place it right after the heap structure
	heap->index = place_ordered_array((void*) start + sizeof(heap_t), HEAP_INDEX_SIZE, (cmpfunc_t) header_t_less_than);

	// Shift the start address forward to resemble where we can start putting data.
	// dont forget heap structure size
	start += sizeof(void*) * HEAP_INDEX_SIZE + sizeof(heap_t);

	// Make sure the start address is page-aligned.
	if (start & 0x00000FFF)
	{
		start &= 0xFFFFF000;
		start += 0x1000;
	}
	// Write the start, end and max addresses into the heap structure.
	heap->start_address = start;
	heap->end_address = end_addr;
	heap->max_address = max;
	heap->supervisor = supervisor;
	heap->readonly = readonly;

	// We start off with one large hole in the index.
	header_t *hole = (header_t *) start;
	hole->size = end_addr - start;
	hole->magic = HEAP_MAGIC;
	hole->is_hole = 1;
	insert_ordered_array((void*) hole, &heap->index);

	return heap;
}

void *alloc(uint64_t size, uint8_t page_align, heap_t *heap)
{

	// Make sure we take the size of header/footer into account.
	uint64_t new_size = size + sizeof(header_t) + sizeof(footer_t);
	// Find the smallest hole that will fit.
	uint64_t iterator = find_smallest_hole(new_size, page_align, heap);
	if ((int64_t)iterator == -1) // If we didn't find a suitable hole
	{
		// Save some previous data.
		uint64_t old_length = heap->end_address - heap->start_address;
		uint64_t old_end_address = heap->end_address;

		// We need to allocate some more space.
		expand(old_length+new_size, heap);
		uint64_t new_length = heap->end_address - heap->start_address;

		// Find the endmost header. (Not endmost in size, but in location).
		iterator = 0;
		// Vars to hold the index of, and value of, the endmost header found so far.
		uint64_t idx = -1; uint64_t value = 0x0;
		while (iterator < heap->index.size)
		{
			uint64_t tmp = (uint64_t)lookup_ordered_array(iterator, &heap->index);
			if (tmp > value)
			{
				value = tmp;
				idx = iterator;
			}
			iterator++;
		}

		// If we didn't find ANY headers, we need to add one.
		if ((int64_t) idx == -1)
		{
			header_t *header = (header_t *)old_end_address;
			header->magic = HEAP_MAGIC;
			header->size = new_length - old_length;
			header->is_hole = 1;
			footer_t *footer = (footer_t *) (old_end_address + header->size - sizeof(footer_t));
			footer->magic = HEAP_MAGIC;
			footer->header = header;
			insert_ordered_array((void*)header, &heap->index);
		}
		else
		{
			// The last header needs adjusting.
			header_t *header = lookup_ordered_array(idx, &heap->index);
			header->size += new_length - old_length;
			// Rewrite the footer.
			footer_t *footer = (footer_t *) ( (uint64_t)header + header->size - sizeof(footer_t) );
			footer->header = header;
			footer->magic = HEAP_MAGIC;
		}
		// We now have enough space. Recurse, and call the function again.
		return alloc(size, page_align, heap);
	}

	header_t *orig_hole_header = (header_t *)lookup_ordered_array(iterator, &heap->index);
	uint64_t orig_hole_pos = (uint64_t)orig_hole_header;
	uint64_t orig_hole_size = orig_hole_header->size;
	// Here we work out if we should split the hole we found into two parts.
	// Is the original hole size - requested hole size less than the overhead for adding a new hole?
	if (orig_hole_size-new_size < sizeof(header_t)+sizeof(footer_t))
	{
		// Then just increase the requested size to the size of the hole we found.
		size += orig_hole_size-new_size;
		new_size = orig_hole_size;
	}

	// If we need to page-align the data, do it now and make a new hole in front of our block.
	if (page_align && (orig_hole_pos + sizeof(header_t)) & 0x00000FFF)
	{
		uint64_t new_location   = orig_hole_pos + MEM_BLOCK_SIZE - (orig_hole_pos&0xFFF) - sizeof(header_t);
		header_t *hole_header = (header_t *)orig_hole_pos;
		hole_header->size     = MEM_BLOCK_SIZE - (orig_hole_pos&0xFFF) - sizeof(header_t);
		hole_header->magic    = HEAP_MAGIC;
		hole_header->is_hole  = 1;
		footer_t *hole_footer = (footer_t *) ( (uint64_t)new_location - sizeof(footer_t) );
		hole_footer->magic    = HEAP_MAGIC;
		hole_footer->header   = hole_header;
		orig_hole_pos         = new_location;
		orig_hole_size        = orig_hole_size - hole_header->size;
	}
	else
	{
		// Else we don't need this hole any more, delete it from the index.
		remove_ordered_array(iterator, &heap->index);
	}

	// Overwrite the original header...
	header_t *block_header  = (header_t *)orig_hole_pos;
	block_header->magic     = HEAP_MAGIC;
	block_header->is_hole   = 0;
	block_header->size      = new_size;
	// ...And the footer
	footer_t *block_footer  = (footer_t *) (orig_hole_pos + sizeof(header_t) + size);
	block_footer->magic     = HEAP_MAGIC;
	block_footer->header    = block_header;

	// We may need to write a new hole after the allocated block.
	// We do this only if the new hole would have positive size...
	if (orig_hole_size - new_size > 0)
	{
		header_t *hole_header = (header_t *) (orig_hole_pos + sizeof(header_t) + size + sizeof(footer_t));
		hole_header->magic    = HEAP_MAGIC;
		hole_header->is_hole  = 1;
		hole_header->size     = orig_hole_size - new_size;
		footer_t *hole_footer = (footer_t *) ( (uint64_t)hole_header + orig_hole_size - new_size - sizeof(footer_t) );
		if ((uint64_t)hole_footer < heap->end_address)
		{
			hole_footer->magic = HEAP_MAGIC;
			hole_footer->header = hole_header;
		}
		// Put the new hole in the index;
		insert_ordered_array((void*)hole_header, &heap->index);
	}

	// ...And we're done!
	return (void *) ( (uint64_t)block_header+sizeof(header_t) );
}

void free(void *p, heap_t *heap)
{
	// Exit gracefully for null pointers.
	if (p == 0)
		return;

	// Get the header and footer associated with this pointer.
	header_t *header = (header_t*) ( (uint64_t) p - sizeof(header_t) );
	footer_t *footer = (footer_t*) ( (uint64_t) header + header->size - sizeof(footer_t) );

	// Sanity checks.
	if (header->magic != HEAP_MAGIC)
		puts("free: header magic bad\n");
	if (footer->magic != HEAP_MAGIC)
		puts("free: footer magic bad\n");

	// Make us a hole.
	header->is_hole = 1;

	// Do we want to add this header into the 'free holes' index?
	char do_add = 1;

	// Unify left
	// If the thing immediately to the left of us is a footer...
	footer_t *test_footer = (footer_t*) ( (uint64_t)header - sizeof(footer_t) );
	if (test_footer->magic == HEAP_MAGIC &&
			test_footer->header->is_hole == 1)
	{
		uint64_t cache_size = header->size; // Cache our current size.
		header = test_footer->header;     // Rewrite our header with the new one.
		footer->header = header;          // Rewrite our footer to point to the new header.
		header->size += cache_size;       // Change the size.
		do_add = 0;                       // Since this header is already in the index, we don't want to add it again.
	}

	// Unify right
	// If the thing immediately to the right of us is a header...
	header_t *test_header = (header_t*) ( (uint64_t)footer + sizeof(footer_t) );
	if (test_header->magic == HEAP_MAGIC &&
			test_header->is_hole)
	{
		header->size += test_header->size; // Increase our size.
		test_footer = (footer_t*) ( (uint64_t)test_header + // Rewrite it's footer to point to our header.
				test_header->size - sizeof(footer_t) );
		footer = test_footer;
		// Find and remove this header from the index.
		uint64_t i = 0;
		while ( (i < heap->index.size) &&
				(lookup_ordered_array(i, &heap->index) != (void*)test_header) )
			i++;

		// Make sure we actually found the item.
		if (i >= heap->index.size)
			puts("free: iterator failed to find item to unify\n");
		// Remove it.
		remove_ordered_array(i, &heap->index);
	}

	// If the footer location is the end address, we can contract.
	if ( (uint64_t)footer + sizeof(footer_t) == heap->end_address)
	{
		uint64_t old_length = heap->end_address - heap->start_address;
		uint64_t new_length = contract( (uint64_t)header - heap->start_address, heap);
		// Check how big we will be after resizing.
		if (header->size - (old_length-new_length) > 0)
		{
			// We will still exist, so resize us.
			header->size -= old_length-new_length;
			footer = (footer_t*) ( (uint64_t)header + header->size - sizeof(footer_t) );
			footer->magic = HEAP_MAGIC;
			footer->header = header;
		}
		else
		{
			// We will no longer exist :(. Remove us from the index.
			uint64_t iterator = 0;
			while ( (iterator < heap->index.size) &&
					(lookup_ordered_array(iterator, &heap->index) != (void*)test_header) )
				iterator++;
			// If we didn't find ourselves, we have nothing to remove.
			if (iterator < heap->index.size)
				remove_ordered_array(iterator, &heap->index);
		}
	}

	// If required, add us to the index.
	if (do_add == 1)
		insert_ordered_array((void*)header, &heap->index);

}
