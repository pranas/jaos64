#include "apic.h"

void disable_legacy_pic()
{
	asm volatile ("movb $0xff, %%al\n\t"
			"out %%al, $0xa1\n\t"
			"out %%al, $0x21"
			::: "rax");
}

void write_apicr(const void* apic_base, const uint8_t offset, const uint32_t val)
{
	/* tell IOREGSEL where we want to write to */
	*(uint32_t*)(apic_base) = offset;
	/* write the value to IOWIN */
	*(uint32_t*)(apic_base + 0x10) = val;
}

uint32_t read_apicr(const void* apic_base, const uint8_t offset)
{
	/* tell IOREGSEL where we want to read from */
	*(uint32_t*)(apic_base) = offset;
	/* return the data from IOWIN */
	return *(uint32_t*)(apic_base + 0x10);
}

void puts_apic_info()
{
	int var = read_apicr(APIC_BASE, 0x20);
	puts("APIC info\n");
	puts("ID: "); puthex(var >> 24); puts("\n");
	var = read_apicr(APIC_BASE, 0x400);
	puts("Version: "); puthex(var & 0x000f); puts("\n");
	puts("Max LVT entries: "); puthex(var & 0x0f00); puts("\n");
	puts("Extended enabled: "); puthex(var >> 32); puts("\n");
}
