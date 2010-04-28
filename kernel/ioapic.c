#include "ioapic.h"

void write_ioapicr(const void* apic_base, const uint8_t offset, const uint32_t val)
{
	/* tell IOREGSEL where we want to write to */
	*(uint32_t*)(apic_base) = offset;
	/* write the value to IOWIN */
	*(uint32_t*)(apic_base + 0x10) = val;
}

uint32_t read_ioapicr(const void* apic_base, const uint8_t offset)
{
	/* tell IOREGSEL where we want to read from */
	*(uint32_t*)(apic_base) = offset;
	/* return the data from IOWIN */
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
		puts("Interrupt "); putint(i); puts(": ");
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
	ioapic_redirect_entry irq0;
	irq0.vector         = 0x21; // vector 0x21
	irq0.delmod         =  0x0; // fixed
	irq0.destmod        =  0x0; // physical
	irq0.intpol         =  0x0; // ??? 0 - high active polarity
	irq0.triggermode    =  0x0; // ??? 0 - edge sensitive
	irq0.mask           =  0x0; // unmask
	irq0.reserved       =  0x0; // zero out reserved
	irq0.reserved2      =  0x0; // zero out reserved
	irq0.destination    =  0x0; // apic id, 0th apic
	write_ioapicr(IOAPIC_BASE, 0x12, irq0.low); // irq1 low
	write_ioapicr(IOAPIC_BASE, 0x13, irq0.high); // irq1 high
}
