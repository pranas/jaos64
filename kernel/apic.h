#ifndef _APIC_H
#define _ACPI_H

#include <stdint.h>

#define APIC_BASE   0xfe000000
#define IOAPIC_BASE 0xfec00000

struct ioapic_redirect_entry
{
	uint64_t vector      :  8;
	uint64_t delmod      :  3;
	uint64_t destmod     :  1;
	uint64_t delivs      :  1;
	uint64_t intpol      :  1;
	uint64_t rirr        :  1;
	uint64_t triggermode :  1;
	uint64_t mask        :  1;
	uint64_t reserved    : 39;
	union {
		uint64_t destination :  8;
		struct {
			uint64_t apicid : 4;
			uint64_t zero : 4;
		};
	};
};



typedef struct ioapic_redirect_entry;

void     write_ioapicr    (const void* apic_base, const uint8_t offset, const uint32_t val);
uint32_t read_ioapicr     (const void* apic_base, const uint8_t offset);
void     puts_ioapic_info ();

#endif
