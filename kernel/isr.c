#include "isr.h"
#include "monitor.h"

void isr_handler(registers_t regs)
{
	puts("Handled some interrupt.\n");
}
