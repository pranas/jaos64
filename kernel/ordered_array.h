#pragma once
#ifndef ORDERED_ARRAY_H
#define ORDERED_ARRAY_H

#include <stdint.h>

typedef int8_t (*cmpfunc_t) (void*, void*);

typedef struct
{
	void* *array;
	uint64_t size;
	uint64_t max_size;
	cmpfunc_t less_than;
} ordered_array_t;

int8_t standard_lessthan_predicate(void* a, void* b);

ordered_array_t create_ordered_array(uint64_t max_size, cmpfunc_t less_than);
ordered_array_t place_ordered_array(void *addr, uint64_t max_size, cmpfunc_t less_than);

void destroy_ordered_array(ordered_array_t *array);
void insert_ordered_array(void* item, ordered_array_t *array);
void* lookup_ordered_array(uint64_t i, ordered_array_t *array);
void remove_ordered_array(uint64_t i, ordered_array_t *array);

#endif
