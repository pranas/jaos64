/*

	Standard library for ProjectOS
	
	Copyright (C) 2010

	It is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2, or (at your option)
	any later version.

*/

#ifndef NULL
#if defined (__cplusplus)
#define NULL 0
#else
#define NULL ((void *)0)
#endif
#endif

/*
	`size_t' is a type suitable for representing the amount
	of memory a data object requires, expressed in units of `char'.
	It is an integer type (C cannot keep track of fractions of a
	`char'), and it is unsigned (negative sizes make no sense).
	It is the type of the result of the `sizeof' operator. It is
	the type you pass to malloc() and friends to say how much
	memory you want. It is the type returned by strlen() to say
	how many "significant" characters are in a string.
	
	<..>
	
	Under LP64, the type definition for size_t changes to long,
 	and the type definition for ptrdiff_t changes to unsigned long. 
*/

#ifndef _SIZE_T
#define _SIZE_T
typedef long size_t;
#endif