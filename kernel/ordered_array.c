
#include "ordered_array.h"

int8_t standard_lessthan_predicate(void* a, void* b)
{
	return (a<b)?1:0;
}

ordered_array_t create_ordered_array(uint32_t max_size, lessthan_predicate_t less_than)
{
	ordered_array_t to_ret;
	to_ret.array = (void*)kmalloc(max_size*sizeof(void*));
	memset(to_ret.array, 0, max_size*sizeof(void*));
	to_ret.size = 0;
	to_ret.max_size = max_size;
	to_ret.less_than = less_than;
	return to_ret;
}

ordered_array_t place_ordered_array(void *addr, uint32_t max_size, lessthan_predicate_t less_than)
{
	ordered_array_t to_ret;
	to_ret.array = (void**)addr;
	memset(to_ret.array, 0, max_size*sizeof(void*));
	to_ret.size = 0;
	to_ret.max_size = max_size;
	to_ret.less_than = less_than;
	return to_ret;
}

void destroy_ordered_array(ordered_array_t *array)
{
	// kfree(array->array);
}

void insert_ordered_array(void* item, ordered_array_t *array)
{
	if (!array->less_than)
		puts("insert_ordered_array: missing less_than\n");
	uint32_t i = 0;
	while (i < array->size && array->less_than(array->array[i], item))
		i++;
	if (i == array->size) // just add at the end of the array.
		array->array[array->size++] = item;
	else
	{
		void* tmp = array->array[i];
		array->array[i] = item;
		while (i < array->size)
		{
			i++;
			void* tmp2 = array->array[i];
			array->array[i] = tmp;
			tmp = tmp2;
		}
		array->size++;
	}
}

void* lookup_ordered_array(uint32_t i, ordered_array_t *array)
{

//	ASSERT(i < array->size);
	if (i < array->size)
		return array->array[i];
	else
		return NULL;
}

void remove_ordered_array(uint32_t i, ordered_array_t *array)
{
	while (i < array->size)
	{
		array->array[i] = array->array[i+1];
		i++;
	}
	array->size--;
} 
