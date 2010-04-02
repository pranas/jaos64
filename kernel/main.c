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

void kernel_entry (multiboot_info* bootinfo) 
{
	clear_screen();
	puts("Hello world!\n");

	//reload gdt's
	gdt_install();
	puts("GDT flushed :)\n");

	// Hardware abstraction layer?
	
	// init interrupt handlers
	
	// parse bootinfo
	
		// get video mem address
		
			// init output
	
		// get mem size and address of BIOS memory map
	
			// initialize mem manager
}
