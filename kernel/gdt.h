#ifndef _GDT_H
#define _GDT_H

struct gdt_entry_struct
{
   int16_t limit_low;           // The lower 16 bits of the limit.
   int16_t base_low;            // The lower 16 bits of the base.
   int8_t  base_middle;         // The next 8 bits of the base.
   int8_t  access;              // Access flags
   int8_t  granularity;
   int8_t  base_high;           // The last 8 bits of the base.
} __attribute__((packed));
typedef struct gdt_entry_struct gdt_entry_struct;

struct gdt_ptr_struct
{
	int16_t size;
	int64_t addr;
} __attribute__((packed));
typedef gdt_ptr_struct gdt_ptr_struct;

#endif

