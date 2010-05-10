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

struct icr_reg
{
	union
	{
		struct
		{
			uint32_t vector  : 8;
			uint32_t msgtype : 3;
			uint32_t destmod : 1;
			uint32_t delstat : 1;
			uint32_t zero1   : 1;
			uint32_t level   : 1;
			uint32_t trigmod : 1;
			uint32_t rrs     : 2;
			uint32_t dsh     : 2;
			uint32_t zero2   :12;
		};
		uint32_t low;
	};
	union
	{
		struct
		{
			uint32_t zero3   :24;
			uint32_t dest    : 8;
		};
		uint32_t high;
	};
};
typedef struct icr_reg icr_reg;

void disable_legacy_pic();
void    write_apicr(uint16_t offset, uint32_t val);
uint32_t read_apicr(uint16_t offset);
void puts_apic_info();
void enable_apic();
void timer_init(int vector, uint32_t counter, uint32_t divider, int periodic);
void apic_init();
void start_ap(uint8_t apicid, uint8_t vector);

#endif
