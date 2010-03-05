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

/*

	To initialize mem manager we need to get pointer to BIOS memory map	
	and mark all available memory as free in our bitmap

*/