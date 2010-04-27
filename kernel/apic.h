#ifndef _APIC_H
#define _APIC_H

#include <stdint.h>

#define APIC_BASE 0xfee00000

void disable_legacy_pic();

void write_apicr(const void* apic_base, const uint8_t offset, const uint32_t val);
uint32_t read_apicr(const void* apic_base, const uint8_t offset);
void puts_apic_info();
#endif
