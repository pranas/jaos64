#ifndef _IDT_H
#define _IDT_H

#include <stdint.h>


struct idt_descriptor
{
	int16_t base_low;  // offset bits 0..15
	int16_t selector;  // a code segment selector in GDT or LDT
     int8_t ist;       // interrupt stack table field 0..2, reserved 3..7
	 int8_t type_attr; // type and attributes, see below
	int16_t base_middle;  // offset bits 16..31
	int32_t base_high;  // offset bits 32..63
	int32_t zero;      // reserved, set to 0
} __attribute__((packed));
typedef struct idt_descriptor idt_descriptor;

struct idt_ptr_struct
{
	int16_t limit;
	int64_t base;
} __attribute__((packed));
typedef struct idt_ptr_struct idt_ptr_struct;

#define IDT_ENTRIES 256
idt_ptr_struct idt_ptr;
idt_descriptor idt[IDT_ENTRIES];

void idt_install();
void idt_flush(idt_ptr_struct* idt_ptr);
void idt_set_gate(int num, int64_t base, int16_t selector, int8_t type, int8_t ist);

/* they are in interrupt.asm 
 * ISR - interrupt service routine */
extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();
extern void isr32();
extern void isr33();
extern void isr128();

#endif
