#include "isr.h"
#include "monitor.h"
#include "apic.h"

isr_t handlers[MAX_HANDLERS];

void isr_handler(registers_t regs)
{
	puts("[ISR] INT ");	puthex(regs.int_no);
	puts(", error code "); puthex(regs.err_code);
	puts("\n");

	if (handlers[regs.int_no] != 0)
	{
		isr_t handler = handlers[regs.int_no];
		handler(&regs);
	}

	if (regs.int_no >= 0x20)
		write_apicr(0xb0, 0); // acknowledge ioapic interrupt
}

void register_handler(int int_no, isr_t handler)
{
	handlers[int_no] = handler;
}
