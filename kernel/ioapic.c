#include "ioapic.h"

#include "memman.h"

void write_ioapicr(const void* apic_base, const uint8_t offset, const uint32_t val)
{
	*(uint32_t*)(apic_base) = offset;
	*(uint32_t*)(apic_base + 0x10) = val;
}

uint32_t read_ioapicr(const void* apic_base, const uint8_t offset)
{
	*(uint32_t*)(apic_base) = offset;
	return *(uint32_t*)(apic_base + 0x10);
}

void puts_ioapic_info()
{
	puts("IOAPIC info => ");
	puts("ID: "); puthex(read_ioapicr(0xfec00000, 0x0)); puts(" Version: "); puthex(read_ioapicr(0xfec00000, 0x1));
   	puts(" AID: "); puthex(read_ioapicr(0xfec00000, 0x2)); puts("\n");
	int i;
	for (i = 0; i < 24; i++)
	{
		puts("IRQ "); putint(i); puts(": ");
		uint32_t lo, hi;
		lo = read_ioapicr(IOAPIC_BASE, 0x10 + 2*i);
		hi = read_ioapicr(IOAPIC_BASE, 0x10 + 2*i + 1);
		puthex(lo);
		puts(" ");
		puthex(hi);
		puts(" ");
		if ((i+1) % 3 == 0)
			puts("\n");
	}
	puts("\n");
}

void ioapic_init()
{
	brute_create_page(0xFEC00000, 0xFEC00000, 1, get_current_pml4(), 0); // IOAPIC address space
	ioapic_redirect_entry irq1;
	irq1.vector         = 0x21; // vector 0x21
	irq1.delmod         =  0x0; // fixed
	irq1.destmod        =  0x0; // physical
	irq1.intpol         =  0x0; // ??? 0 - high active polarity
	irq1.triggermode    =  0x0; // ??? 0 - edge sensitive
	irq1.mask           =  0x0; // unmask
	irq1.reserved       =  0x0; // zero out reserved
	irq1.reserved2      =  0x0; // zero out reserved
	irq1.destination    =  0x0; // apic id, 0th apic
	write_ioapicr(IOAPIC_BASE, 0x12, irq1.low); // irq1 low
	write_ioapicr(IOAPIC_BASE, 0x13, irq1.high); // irq1 high

	puts_ioapic_info();
}
