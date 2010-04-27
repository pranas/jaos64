#include "isr.h"
#include "monitor.h"
#include "apic.h"

void isr_handler(registers_t regs)
{
	puts("Handled interrupt ");
	puthex(regs.int_no);
	puts(".\n");
	if (regs.int_no == 0x20)
		write_apicr(APIC_BASE, 0xb0, 0);
}
