#ifndef _APIC_H
#define _APIC_H

#include <stdint.h>

#define APIC_BASE 0xfee00000

struct lvt_entry
{
	int32_t vector    : 8;
	int32_t msgtype   : 3;
	int32_t reserved  : 1;
	int32_t delstat   : 1;
	int32_t reserved2 : 1;
	int32_t rirr      : 1;
	int32_t trigmode  : 1;
	int32_t mask      : 1;
	int32_t timermode : 1;
	int32_t reserved3 : 14;
};
typedef struct lvt_entry lvt_entry;

void disable_legacy_pic();

void write_apicr(uint32_t* apic_base, uint16_t offset, uint32_t val);
uint32_t read_apicr(uint32_t* apic_base, uint16_t offset);
void puts_apic_info();
void enable_apic();

#endif
