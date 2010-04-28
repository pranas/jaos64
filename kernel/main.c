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
#include "apic.h"
#include "ioapic.h"
#include "fat32.h"

void kernel_entry (multiboot_info* bootinfo) 
{
	clear_screen();	puts("Kernel loaded.\n");
	gdt_install();  puts("GDT initialised.\n");
	idt_install();	puts("IDT initialised.\n");
    memman_init(bootinfo);
	// init fat32
	fat32_init();

	acpi_init();
	apic_init();
	ioapic_init(); // keyboard only for now
	init_timer(0x20, 0x00ffffff, 0xB, 1); // vector, counter, divider, periodic -- check manual before using
	asm ("sti"); // release monsters
	asm ("xchg %bx, %bx");
	for (;;);
}
