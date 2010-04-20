/*

	memman.c
	
	Memory manager

	Copyright (C) 2010

	This is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2, or (at your option)
	any later version.

*/

#include "memman.h"
#include <bootinfo.h>
#include <ctype.h>

/*

	To initialize mem manager we need to get pointer to BIOS memory map	
	and mark all available memory as free in our bitmap

*/

void memman_init(multiboot_info* bootinfo)
{
    char* strMemoryType[] = { "Unknown", "Available", "Reserved", "ACPI Reclaim", "ACPI NVS Memory", "Bad" };
    memory_region *memory_map = 0;
    memory_map = bootinfo->m_mmap_addr;
    int i, total;
    putchar(bootinfo->m_mmap_length + 48);
    puts(" entries found\n");
    for (i = 0; i < bootinfo->m_mmap_length; i++)
    {
        if (memory_map[i].type == 1) total = total + memory_map[i].size;
        
        puts("From: ");
        puthex(memory_map[i].start);
        puts("    Size: ");
        puthex(memory_map[i].size);
        puts("     Type:");
        puts(strMemoryType[memory_map[i].type]);
        puts("\n");
    }
    
    puts("\n");
    puts("Total memory found: ");
    putint(total);
    putchar("B");    
}