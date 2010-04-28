#include "apic.h"

#include <string.h>

#include "memman.h"
#include "io.h"

void disable_legacy_pic()
{
	outb(0xa1, 0xff);
	outb(0x21, 0xff);
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
	puts("[APIC] ID: "); puthex(var >> 24); puts("\n");

	// NOT IMPLEMENTED ON BOCHS T_T
	//var = read_apicr(APIC_BASE, 0x0400);
	//puts("Version: "); puthex(var & 0x000f); puts("\n");
	//puts("Max LVT entries: "); puthex(var & 0x0f00); puts("\n");
	//puts("Extended enabled: "); puthex(var >> 32); puts("\n");
}

void puts_apic_register(int offset)
{
	puts("[APIC] APIC register "); puthex(offset); puts(": ");
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

void apic_init()
{
	disable_legacy_pic();
	brute_create_page(APIC_BASE, APIC_BASE, 1, get_current_pml4(), 0); // APIC address space
	enable_apic(); // even though its already enabled :S
	write_apicr(APIC_BASE, 0xf0, 0x00000100); // spurious int register
	puts_apic_info();
}

void init_timer(int vector, uint32_t counter, uint32_t divider, int periodic)
{
	uint32_t timer_lvt = 0x0;
	timer_lvt |= vector;
	if (periodic)
		timer_lvt |= 0x0020000;

	write_apicr(APIC_BASE, 0x3e0, divider); // divider
	write_apicr(APIC_BASE, 0x320, timer_lvt); // persistent
	write_apicr(APIC_BASE, 0x380, counter); // counter
	puts("[APIC] Local timer initialised.\n");
}
