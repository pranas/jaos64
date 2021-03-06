/*

	string.h

	Standard C String routines
	
	Copyright (C) 2010

	This is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2, or (at your option)
	any later version.

*/

#ifndef _STRING_H
#define _STRING_H

#include <stdlib.h>

size_t strlen (const char* str);
int strncmp (const char*, const char*, size_t);
int strcmp (const char*, const char*);
char *strncpy(char *dest, const char *src, size_t n);

//extern char *strcpy(char *s1, const char *s2);
//extern void* memcpy(void *dest, const void *src, size_t count);
//extern void *memset(void *dest, char val, size_t count);
//extern unsigned short* memsetw(unsigned short *dest, unsigned short val, size_t count);

#endif
