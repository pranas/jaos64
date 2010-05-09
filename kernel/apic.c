#include "apic.h"

#include <string.h>

#include "monitor.h"
#include "msr.h"
#include "memman.h"
#include "io.h"

void disable_legacy_pic()
{
	outb(0xa1, 0xff);
	outb(0x21, 0xff);
}

void write_apicr(uint16_t offset, uint32_t val)
{
	uint32_t * addr = (uint32_t*) ((char*) APIC_BASE + offset);
	*addr = val;
}

uint32_t read_apicr(uint16_t offset)
{
	uint32_t* addr = (uint32_t*) ((char*) APIC_BASE + offset);
	return *addr;
}

void puts_apic_info()
{
	int var = read_apicr(0x20);
	puts("[APIC] ID: "); puthex(var >> 24); puts("\n");

	// NOT IMPLEMENTED ON BOCHS T_T
	//var = read_apicr(0x0400);
	//puts("Version: "); puthex(var & 0x000f); puts("\n");
	//puts("Max LVT entries: "); puthex(var & 0x0f00); puts("\n");
	//puts("Extended enabled: "); puthex(var >> 32); puts("\n");
}

void puts_apic_register(int offset)
{
	puts("[APIC] APIC register "); puthex(offset); puts(": ");
	puthex(read_apicr(offset));
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
	brute_create_page(APIC_BASE, APIC_BASE, 1, (void*) get_current_pml4(), 0); // APIC address space
	enable_apic(); // even though its already enabled :S
	write_apicr(0xf0, 0x00000100); // spurious int register
	puts_apic_info();
}

void timer_init(int vector, uint32_t counter, uint32_t divider, int periodic)
{
	uint32_t timer_lvt = 0x0;
	timer_lvt |= vector;
	if (periodic)
		timer_lvt |= 0x0020000;

	write_apicr(0x3e0, divider); // divider
	write_apicr(0x320, timer_lvt); // persistent
	write_apicr(0x380, counter); // counter
	puts("[APIC] Local timer initialised.\n");
}

void start_ap(uint8_t apicid, uint8_t vector)
{
	/* AP startup code, starts at 0x9000 */
	icr_reg startupipi;
	startupipi.vector  = 0x0;
	startupipi.msgtype = 0x5;
	startupipi.destmod = 0x0;
	startupipi.level   = 0x0;
	startupipi.trigmod = 0x0;
	startupipi.dsh     = 0x0;
	startupipi.dest    = apicid;
	write_apicr(0x310, startupipi.high);
	write_apicr(0x300, startupipi.low);
	startupipi.vector  = vector;
	startupipi.msgtype = 0x6;
	startupipi.destmod = 0x0;
	startupipi.level   = 0x0;
	startupipi.trigmod = 0x0;
	startupipi.dsh     = 0x0;
	startupipi.dest    = apicid;
	write_apicr(0x310, startupipi.high);
	write_apicr(0x300, startupipi.low);
}
