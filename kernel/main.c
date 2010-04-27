/*

	main.c

	Copyright (C) 2010

	This is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2, or (at your option)
	any later version.

*/

#include <bootinfo.h>
#include "monitor.h"
#include "gdt.h"
#include "idt.h"
#include "msr.h"
#include "memman.h"

void kernel_entry (multiboot_info* bootinfo) 
{
	clear_screen();
	puts("Hello world!\n");

	gdt_install();
	puts("GDT initialised.\n");

	idt_install();
	puts("IDT initialised.\n");
    // parse bootinfo
    memman_init(bootinfo);
	brute_create_page(0xFEC00000, 0xFEC00000, 0x20, get_current_pml4(), 0);
	puts_ioapic_info();
	asm ("xchg %bx, %bx");

}
