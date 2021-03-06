#include "io.h"

void outb(int16_t port, int8_t value)
{
	asm volatile ("outb %1, %0"
			:
			: "dN" (port), "a" (value)); //inputs
}

int8_t inb(int16_t port)
{
	int8_t ret;
	asm volatile ("inb %1, %0"
			: "=a" (ret) //outputs
			: "dN" (port)); //inputs
	return ret;
}

int16_t inw(int16_t port)
{
	int16_t ret;
	asm volatile ("inw %1, %0"
			: "=a" (ret) //outputs
			: "dN" (port)); //inputs
	return ret;
}

