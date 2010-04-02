#ifndef _GDT_H
#define _GDT_H

#include <stdint.h>
/* GDT entry */
struct gdt_entry_struct
{
   int16_t limit_low;           // The lower 16 bits of the limit.
   int16_t base_low;            // The lower 16 bits of the base.
   int8_t  base_middle;         // The next 8 bits of the base.
   int8_t  access;              // Access flags
   int8_t  granularity;         // with 4 bits of limit
   int8_t  base_high;           // The last 8 bits of the base.
} __attribute__((packed));
typedef struct gdt_entry_struct gdt_entry_struct;

/* GDT table pointer */
struct gdt_ptr_struct
{
	int16_t limit;
	int64_t base;
} __attribute__((packed));
typedef struct gdt_ptr_struct gdt_ptr_struct;

#define GDT_ENTRY_NR 3
struct gdt_ptr_struct gdt_ptr;
struct gdt_entry_struct gdt[GDT_ENTRY_NR];

/* this one is in gdt_flush.asm */
extern void gdt_flush();

void gdt_install();
void gdt_set_gate(int num, int32_t base, int32_t limit, int8_t access, int8_t granularity);

#endif

