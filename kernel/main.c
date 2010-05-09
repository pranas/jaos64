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
#include "isr.h"
#include "msr.h"
#include "memman.h"
#include "acpi.h"
#include "apic.h"
#include "ioapic.h"
#include "fat32.h"
#include "keyboard.h"
#include "scheduler.h"
#include "syscall.h"
#include "kheap.h"

void kernel_entry (multiboot_info* bootinfo) 
{
	clear_screen();	puts("Kernel loaded.\n");
	gdt_install();  puts("GDT initialised.\n");
	idt_install();	puts("IDT initialised.\n");
    memman_init(bootinfo);
	fat32_init();

	// acpi_init();
	apic_init();
	ioapic_init(); // keyboard only for now

	register_handler(0x21, keyboard_handler);

	init_syscalls(); // maybe syscalls_init() like acpi_init, apic_init, etc... there should be common naming

	init_timer(0x20, 0x02ffffff, 0xB, 1); // vector, counter, divider, periodic -- check manual before using

	// sets up kernel task and registers handler for timer
	/*
	scheduler_init();

	// testing scheduler
	if (fork_kernel() == 0)
	{
		for(;;)
		{
			puts("PONG!\n\n");
			asm volatile("hlt");
		}
	}
	else
	{
		for(;;)
		{
			puts("PING!\n\n");
			asm volatile("hlt");
		}
	}
	*/
	int i;
	for (i = 0; i < 0x10000; i++)
	{
		void* mem = kmalloc(sizeof(int));
		*(int*) mem = i;
	}
	asm ("xchg %bx, %bx");
	
	asm ("sti"); // release monsters, it can be set earlier, but fails horribly if set before acpi_init

	for (;;);
}
