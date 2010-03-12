#include <stdint.h>

struct idt_descriptor
{
	int16_t offset_1; // offset bits 0..15
	int16_t selector; // a code segment selector in GDT or LDT
	int8_t zero1;     // unused, set to 0
	int8_t type_attr; // type and attributes, see below
	int16_t offset_2; // offset bits 16..31
	int64_t offset_3; // offset bits 32..63
	int8_t zero2;     // unused, set to 0
	int8_t zero3;     // bits 0..4 should be zeroed out
	int16_t zero4;    // zero out
} __attribute__((packed));

struct idt_ptr_struct
{
	int16_t size;
	void* addr;
} __attribute__((packed));

void load_idtr(struct idt_ptr_struct* idt_ptr);
void set_idt(int offset, struct idt_descriptor entry);

