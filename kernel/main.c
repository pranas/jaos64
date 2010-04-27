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
#include "fat32.h"

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
	// init fat32
	fat32_init();
	
	asm ("int $0x3");

	asm ("xchg %bx, %bx");

	int low, high;
	rdmsr(0x1B, &low, &high);
	if (low & 0x0800)
		puts("Local APIC enabled.\n");
}
