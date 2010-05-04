#include "gdt.h"

void gdt_set_gate(int num, int32_t base, int32_t limit, int8_t access, int8_t granularity)
{
	/* base address */
	gdt[num].base_low     = (base & 0xFFFF);
	gdt[num].base_middle  = (base >> 16) & 0xFF;
	gdt[num].base_high    = (base >> 24) & 0xFF;

	/* limit */
	gdt[num].limit_low    = (limit & 0xFFFF);
	gdt[num].granularity  = (limit >> 16) & 0x0F;

	/* granularity */
	gdt[num].granularity |= (granularity & 0xF0);
	gdt[num].access       = access;
}

void gdt_install()
{
	gdt_ptr.limit = (sizeof(gdt_entry_struct) * GDT_ENTRY_NR) - 1;
	gdt_ptr.base  = (int64_t) &gdt;

	// NULL gate
	gdt_set_gate(0, 0, 0, 0, 0);

	// kernel space
	gdt_set_gate(1, 0, 0, 0x98, 0x20); // code
	gdt_set_gate(2, 0, 0, 0x92, 0x00); // data

	// user space
	gdt_set_gate(3, 0, 0, 0xF8, 0x20); // code
	gdt_set_gate(4, 0, 0, 0xF2, 0x00); // data

	//apply changes
	gdt_flush();
}
