#include "isr.h"
#include "monitor.h"
#include "apic.h"

#include "keyboard.h"

void isr_handler(registers_t regs)
{
	puts("[ISR] INT ");
	puthex(regs.int_no);
	puts(", error code ");
	puthex(regs.err_code);
	puts("\n");

	if (regs.int_no == 0x20)
	{
		//local_timer_handler();
	}
	if (regs.int_no == 0x21) // keyboard
	{
		keyboard_handler();
	}

	if (regs.int_no >= 0x20)
		write_apicr(APIC_BASE, 0xb0, 0); // acknowledge ioapic interrupt
}
