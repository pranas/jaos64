#pragma once
#ifndef IOAPIC_H
#define IOAPIC_H

#include <stdint.h>
#include "memman.h"
#include "monitor.h"

#define IOAPIC_BASE 0xfec00000

struct ioapic_version_table
{
	int8_t apicver;
	int8_t reserved;
	int8_t maximumredentry;
	int8_t reserved2;
};
typedef struct ioapic_version_table ioapic_version_table;

struct ioapic_redirect_entry
{
	union
	{
		struct
		{
			uint64_t vector      :  8; // interrupt vector
			uint64_t delmod      :  3; // delivery mode
			uint64_t destmod     :  1; // destination mode, 0 - physical, 1 - logical
			uint64_t delivs      :  1; // delivery status
			uint64_t intpol      :  1; // interrupt input pin polarity
			uint64_t rirr        :  1; // remote IRR
			uint64_t triggermode :  1; // trigger mode, 0 - edge sensitive, 1 - level sensitive
			uint64_t mask        :  1; // mask
			uint64_t reserved    : 15; // 0
		};
		uint32_t low;
	};
	union
	{
		struct
		{
			uint64_t reserved2   : 24; // 0
			uint64_t destination :  8; // if destmod == 1
		};
		uint32_t high;
	};
};
typedef struct ioapic_redirect_entry ioapic_redirect_entry;

void     write_ioapicr    (const uint8_t offset, const uint32_t val);
uint32_t read_ioapicr     (const uint8_t offset);
void     puts_ioapic_info ();
void     ioapic_init();

#endif
