#pragma once
#ifndef GDT_H
#define GDT_H

#include <stdint.h>
#include "monitor.h"
#include "stdlib.h"

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

struct system_segment_descriptor
{
   int16_t  limit_low;           // The lower 16 bits of the limit.
   int16_t  base_low;            // The lower 16 bits of the base.
   int8_t   base_middle;
   int8_t   access;              // Access flags
   int8_t   granularity;
   int8_t   base_high;
   uint32_t base_veryhigh;
   uint32_t zero;
} __attribute__((packed));
typedef struct system_segment_descriptor system_segment_descriptor;

/* GDT table pointer */
struct gdt_ptr_struct
{
	int16_t limit;
	int64_t base;
} __attribute__((packed));
typedef struct gdt_ptr_struct gdt_ptr_struct;

struct tss_entry_t
{
	uint32_t zero1;
	uint32_t rsp0_low;
	uint32_t rsp0_high;
	uint32_t rsp1_low;
	uint32_t rsp1_high;
	uint32_t rsp2_low;
	uint32_t rsp2_high;
	uint64_t zero2;
	uint32_t ist1_low;
	uint32_t ist1_high;
	uint32_t ist2_low;
	uint32_t ist2_high;
	uint32_t ist3_low;
	uint32_t ist3_high;
	uint32_t ist4_low;
	uint32_t ist4_high;
	uint32_t ist5_low;
	uint32_t ist5_high;
	uint32_t ist6_low;
	uint32_t ist6_high;
	uint32_t ist7_low;
	uint32_t ist7_high;
	uint64_t zero3;
	uint16_t zero4;
	uint16_t iomap_adr;
};

typedef struct tss_entry_t tss_entry_t;
#define GDT_ENTRY_NR 32
struct gdt_ptr_struct gdt_ptr;
struct gdt_entry_struct gdt[GDT_ENTRY_NR];
tss_entry_t tss_entry;

/* this one is in gdt_flush.asm */
extern void gdt_flush();

void gdt_install();
void gdt_set_gate(int num, int32_t base, int32_t limit, int8_t access, int8_t granularity);
// for system segment descriptors (TSS)
void gdt_set_ss_gate(int num, int64_t base, int32_t limit, int8_t access, int8_t granularity);
void tss_set_gate(int num, uint64_t rsp0, uint64_t rsp1, uint64_t rsp2);
void load_tr(int num);

#endif

