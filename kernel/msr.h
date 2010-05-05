#ifndef _MSR_H
#define _MSR_H

#include "stdint.h"

void rdmsr(uint32_t msr, uint32_t* low, uint32_t* high);
void wrmsr(uint32_t msr, uint32_t low, uint32_t high);

#endif
