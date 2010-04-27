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
	puts("IOAPIC ID: "); puthex(read_ioapicr(0xfec00000, 0x0)); puts("\n");
	puts("IOAPIC version: "); puthex(read_ioapicr(0xfec00000, 0x1)); puts("\n");
	puts("IOAPIC AID: "); puthex(read_ioapicr(0xfec00000, 0x2)); puts("\n");
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
	}
	puts("\n");
}
