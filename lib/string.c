/*

	string.c

	Standard C String routines

	Copyright (C) 2010

	This is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2, or (at your option)
	any later version.

*/

#include <string.h>

size_t strlen (const char* str) {
	size_t len = 0;
	while (str[len++]);
	return len;
}