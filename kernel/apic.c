#include "apic.h"

void disable_legacy_pic()
{
	asm volatile ("movb $0xff, %%al\n\t"
			"out %%al, $0xa1\n\t"
			"out %%al, $0x21"
			::: "rax");
}

void write_apicr(uint32_t* apic_base, uint16_t offset, uint32_t val)
{
	uint32_t* addr = (char*) apic_base + offset;
	*addr = val;
}

uint32_t read_apicr(uint32_t* apic_base, uint16_t offset)
{
	uint32_t* addr = (char*) apic_base + offset;
	return *addr;
}

void puts_apic_info()
{
	int var = read_apicr(APIC_BASE, 0x20);
	puts("APIC info => "); puts("ID: "); puthex(var >> 24); puts("\n");

	// NOT IMPLEMENTED ON BOCHS T_T
	//var = read_apicr(APIC_BASE, 0x0400);
	//puts("Version: "); puthex(var & 0x000f); puts("\n");
	//puts("Max LVT entries: "); puthex(var & 0x0f00); puts("\n");
	//puts("Extended enabled: "); puthex(var >> 32); puts("\n");
}

void puts_apic_register(int offset)
{
	puts("APIC register "); puthex(offset); puts(": ");
	puthex(read_apicr(APIC_BASE, offset));
	puts("\n");
}

void enable_apic()
{
	uint32_t lo = 0, hi = 0;
	rdmsr(0x1B, &lo, &hi);
	lo |= 0x00000800;
	wrmsr(0x1B, lo, hi);
}
