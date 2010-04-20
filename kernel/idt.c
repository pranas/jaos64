#include <stdlib.h>

#include "idt.h"

void idt_flush(idt_ptr_struct* idt_p)
{
	asm volatile ("lidt %0"
				  : "=m" (*idt_p)
				  : "m" (*idt_p));
}

void idt_set_gate(int num, int64_t base, int16_t selector, int8_t type, int8_t ist)
{
	// Base address
	idt[num].base_low    = (base & 0x0FFFF);
	idt[num].base_middle = ((base >> 16) & 0x0FFFF);
	idt[num].base_high   = ((base >> 32) & 0x0FFFFFFFF);

	// other fields
	idt[num].selector    = selector;
	idt[num].type_attr   = type;
	idt[num].ist         = (ist & 0x07); // zero out reserved bits
}

void idt_install()
{
	idt_ptr.base  = (int64_t) &idt;
	idt_ptr.limit = sizeof(idt_descriptor) * IDT_ENTRIES - 1;

	memset(&idt, 0, sizeof(idt_descriptor) * IDT_ENTRIES);

	asm volatile ("xchg %bx, %bx");
	idt_set_gate(0, (int64_t) isr0, 0x08, 0x8E, 0);
	idt_set_gate(1, (int64_t) isr1, 0x08, 0x8E, 0);
	idt_set_gate(2, (int64_t) isr2, 0x08, 0x8E, 0);
	idt_set_gate(3, (int64_t) isr3, 0x08, 0x8E, 0);
	idt_set_gate(4, (int64_t) isr4, 0x08, 0x8E, 0);
	idt_set_gate(5, (int64_t) isr5, 0x08, 0x8E, 0);
	idt_set_gate(6, (int64_t) isr6, 0x08, 0x8E, 0);
	idt_set_gate(7, (int64_t) isr7, 0x08, 0x8E, 0);
	idt_set_gate(8, (int64_t) isr8, 0x08, 0x8E, 0);
	idt_set_gate(9, (int64_t) isr9, 0x08, 0x8E, 0);
	idt_set_gate(10, (int64_t)isr10, 0x08, 0x8E, 0);
	idt_set_gate(11, (int64_t)isr11, 0x08, 0x8E, 0);
	idt_set_gate(12, (int64_t)isr12, 0x08, 0x8E, 0);
	idt_set_gate(13, (int64_t)isr13, 0x08, 0x8E, 0);
	idt_set_gate(14, (int64_t)isr14, 0x08, 0x8E, 0);
	idt_set_gate(15, (int64_t)isr15, 0x08, 0x8E, 0);
	idt_set_gate(16, (int64_t)isr16, 0x08, 0x8E, 0);
	idt_set_gate(17, (int64_t)isr17, 0x08, 0x8E, 0);
	idt_set_gate(18, (int64_t)isr18, 0x08, 0x8E, 0);
	idt_set_gate(19, (int64_t)isr19, 0x08, 0x8E, 0);
	idt_set_gate(20, (int64_t)isr20, 0x08, 0x8E, 0);
	idt_set_gate(21, (int64_t)isr21, 0x08, 0x8E, 0);
	idt_set_gate(22, (int64_t)isr22, 0x08, 0x8E, 0);
	idt_set_gate(23, (int64_t)isr23, 0x08, 0x8E, 0);
	idt_set_gate(24, (int64_t)isr24, 0x08, 0x8E, 0);
	idt_set_gate(25, (int64_t)isr25, 0x08, 0x8E, 0);
	idt_set_gate(26, (int64_t)isr26, 0x08, 0x8E, 0);
	idt_set_gate(27, (int64_t)isr27, 0x08, 0x8E, 0);
	idt_set_gate(28, (int64_t)isr28, 0x08, 0x8E, 0);
	idt_set_gate(29, (int64_t)isr29, 0x08, 0x8E, 0);
	idt_set_gate(30, (int64_t)isr30, 0x08, 0x8E, 0);
	idt_set_gate(31, (int64_t)isr31, 0x08, 0x8E, 0);

	idt_flush(&idt_ptr);
}

void sti()
{
	asm volatile ("sti");
}
