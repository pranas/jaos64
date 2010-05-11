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

size_t strlen (const char* str)
{
	size_t len = 0;
	while (str[len++]);
	return len;
}

int strncmp (const char* str, const char* str2, size_t n)
{
	int i;
	for (i = 0; i < n; i++)
	{
		if ((str[i] == 0) && (str2[i] == 0))
			return 1;
		if ((str[i] == 0) || (str2[i] == 0))
			return 0;
		if (str[i] != str2[i])
			return 0;
	}
	return 1;
}

int strcmp (const char* str, const char* str2)
{
        int i = 0;
        while (str[i] == str2[i])
        {
                if ((str[i] == 0) && (str2[i] == 0))
                        return 1;
                if ((str[i] == 0) || (str2[i] == 0))
                        return 0;
				i++;
        }
        return 0;
}

char *strncpy(char *dest, const char *src, size_t n)
{
	int i;
	for (i = 0; i < n && src[i] != '\0'; i++)
		dest[i] = src[i];
	for (; i < n; i ++)
		dest[i] = '\0';

	return dest;
}
