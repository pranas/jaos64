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
	gdt_ptr.limit = (sizeof(gdt_ptr_struct) * GDT_ENTRY_NR) - 1;
	gdt_ptr.base  = (int64_t) &gdt;

	// NULL gate
	gdt_set_gate(0, 0, 0, 0, 0);
	//code
	//gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
	gdt_set_gate(1, 0, 0, 0x98, 0x20);
	//data
	//gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);
	gdt_set_gate(2, 0, 0, 0x92, 0x00);

	//apply changes
	gdt_flush();
}
