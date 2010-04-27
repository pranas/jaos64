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
	clear_screen();
	puts("Hello world!\n");

	gdt_install();
	puts("GDT initialised.\n");

	idt_install();
	puts("IDT initialised.\n");

    memman_init(bootinfo);

	// init fat32
	fat32_init();
	asm ("sti"); // release monsters
	
	brute_create_page(0xFEE00000, 0xFEE00000, 1, get_current_pml4(), 0); // APIC address space
	brute_create_page(0xFEC00000, 0xFEC00000, 1, get_current_pml4(), 0); // IOAPIC address space
	disable_legacy_pic();
	enable_apic(); // even though its already enabled :S
	puts_apic_info();
	puts_ioapic_info();

	write_apicr(APIC_BASE, 0xf0, 0x00000100); 
	write_apicr(APIC_BASE, 0x3e0, 0x0000000B); // divider
	write_apicr(APIC_BASE, 0x320, 0x00020020); // persistent
	write_apicr(APIC_BASE, 0x380, 0x00ffffff); // counter

	asm ("xchg %bx, %bx");
	for (;;);
}
