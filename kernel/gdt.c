#include "gdt.h"

#include "monitor.h"
#include "stdlib.h"

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

void gdt_set_ss_gate(int num, int64_t base, int32_t limit, int8_t access, int8_t granularity)
{
	/* base address */
	gdt[num].base_low    = (base & 0xFFFF);
	gdt[num].base_middle = (base >> 16) & 0xFF;
	gdt[num].base_high   = (base >> 24) & 0xFF;

	gdt[num+1].base_low = (base >> 48) & 0xFFFF;
	gdt[num+1].limit_low = (base >> 32) & 0x0000FFFF;
	gdt[num+1].base_middle = 0;
	gdt[num+1].access = 0;
	gdt[num+1].granularity = 0;
	gdt[num+1].base_high = 0;

	gdt[num].limit_low    = (limit & 0xFFFF);
	gdt[num].granularity  = (limit >> 16) & 0x0F;

	gdt[num].granularity |= (granularity & 0xF0);
	gdt[num].access       = access;

}

void gdt_install()
{
	gdt_ptr.limit = (sizeof(gdt_entry_struct) * GDT_ENTRY_NR) - 1;
	putint(gdt_ptr.limit);
	gdt_ptr.base  = (int64_t) &gdt;

	// NULL gate
	gdt_set_gate(0, 0, 0, 0, 0);

	// kernel space
	gdt_set_gate(1, 0, 0, 0x98, 0x20); // code
	gdt_set_gate(2, 0, 0, 0x92, 0x00); // data

	// user space
	gdt_set_gate(3, 0, 0, 0xF8, 0x20); // code
	gdt_set_gate(4, 0, 0, 0xF2, 0x00); // data

	// try to add a tss gate
	tss_set_gate(5, 0x90000, 0x90000, 0x90000);

	// apply changes
	gdt_flush();

	// must be after gdt is loaded
	load_tr(5);
}

void load_tr(int num)
{
	int sel = num * 8;
	asm ("ltr %%ax" :: "a" (sel));
}

void tss_set_gate(int num, uint64_t rsp0, uint64_t rsp1, uint64_t rsp2)
{
	uint64_t base = (uint64_t) &tss_entry;
	uint64_t limit = base + sizeof(tss_entry);

	gdt_set_ss_gate(num, base, limit, 0x89, 0x9F);
	memset(&tss_entry, 0, sizeof(tss_entry));

	tss_entry.rsp0_high = (rsp0 >> 32);
	tss_entry.rsp0_low = rsp0 & 0xFFFFFFFF;
	tss_entry.rsp1_high = (rsp1 >> 32);
	tss_entry.rsp1_low = rsp1 & 0xFFFFFFFF;
	tss_entry.rsp2_high = (rsp2 >> 32);
	tss_entry.rsp2_low = rsp2 & 0xFFFFFFFF;
}
