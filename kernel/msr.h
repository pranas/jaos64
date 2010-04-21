#ifndef _MSR_H
#define _MSR_H

#include "stdint.h"

void rdmsr(int32_t msr, int32_t* low, int32_t* high);
void wrmsr(int32_t msr, int32_t low, int32_t high);

#endif
