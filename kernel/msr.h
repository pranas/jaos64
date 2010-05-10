#pragma once
#ifndef MSR_H
#define MSR_H

#include "stdint.h"

void rdmsr(uint32_t msr, uint32_t* low, uint32_t* high);
void wrmsr(uint32_t msr, uint32_t low, uint32_t high);

#endif
