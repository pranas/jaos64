#include <stdlib.h>

#include "idt.h"

void load_idt(struct idt_ptr_struct* idt_ptr)
{
	asm volatile ("lidt %0" : : "a" (idt_ptr));
}

void set_idt(int offset, struct idt_descriptor *entry)
{
	struct idt_ptr_struct *idt_addr;
	asm volatile ("sidt %0" : "=a" (idt_addr)); 
	// "=" means its write-only, "m" memory operand
	memcpy(idt_addr + offset, entry);
}
