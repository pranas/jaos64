#include "isr.h"
#include "monitor.h"

void isr_handler(registers_t regs)
{
	puts("Handled interrupt ");
	puthex(regs.int_no);
	puts(".\n");
}
