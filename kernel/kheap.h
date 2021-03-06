#pragma once
#ifndef KHEAP_H
#define KHEAP_H

#include <stdint.h>
#include "ordered_array.h"
#include "memman.h"

// TODO: make this more fool proof
// if initial_size - index_size * 8 + 1 page < min_size
// errors come up
#define KHEAP_INITIAL_SIZE  0x20000
#define HEAP_INDEX_SIZE   0x1000
#define HEAP_MAGIC        0xDEADBABA
#define HEAP_MIN_SIZE     0x10000

struct header_t
{
	uint32_t magic;   // Magic number, used for error checking and identification.
	uint8_t is_hole;   // 1 if this is a hole. 0 if this is a block.
	uint64_t size;    // size of the block, including the end footer.
};
typedef struct header_t header_t;

struct footer_t
{
	uint32_t magic;     // Magic number, same as in header_t.
	header_t *header; // Pointer to the block header.
};
typedef struct footer_t footer_t;

struct heap_t
{
	ordered_array_t index;
	uint64_t start_address; // The start of our allocated space.
	uint64_t end_address;   // The end of our allocated space. May be expanded up to max_address.
	uint64_t max_address;   // The maximum address the heap can be expanded to.
	uint8_t supervisor;     // Should extra pages requested by us be mapped as supervisor-only?
	uint8_t readonly;       // Should extra pages requested by us be mapped as read-only?
};
typedef struct heap_t heap_t;

heap_t *create_heap(uint64_t start, uint64_t end, uint64_t max, uint8_t supervisor, uint8_t readonly);
void *alloc(uint64_t size, uint8_t page_align, heap_t *heap);
void free(void *p, heap_t *heap); 

uint64_t kmalloc_int(uint64_t sz, int align, uint64_t *phys);
uint64_t kmalloc(uint64_t sz);
uint64_t kmalloc_a(uint64_t sz);
uint64_t kmalloc_p(uint64_t sz, uint64_t *phys);
uint64_t kmalloc_ap(uint64_t sz, uint64_t *phys);
void kfree(void *p);
void kheap_init();

#endif
