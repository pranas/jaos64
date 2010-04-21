#include "msr.h"

void rdmsr(int32_t msr, int32_t* low, int32_t* high)
{
	asm volatile ("rdmsr" : "=a" (*low), "=d" (*high) : "c" (msr));
}

void wrmsr(int32_t msr, int32_t low, int32_t high)
{
	asm volatile ("wrmsr" :: "a" (low), "d" (high), "c" (msr));
}

