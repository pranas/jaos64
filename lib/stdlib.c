#include <stdlib.h>

void* memset(void *destaddr, int c, size_t n)
{
	char * dest = destaddr;
	int i;
	for (i = 0; i < n; i++)
	{
		dest[i] = c;
	}
}

void* memcpy(void *destaddr, const void *srcaddr, size_t n)
{
	char * dest = (char*) destaddr;
	char * src  = (char*) srcaddr;
	int i;
	for (i = 0; i < n; i++)
	{
		dest[i] = src[i];
	}
	return dest;
}

void* memmove(void *destaddr, const void *srcaddr, size_t n)
{
	char * dest = (char*) destaddr;
	char *  src = (char*) srcaddr;
	int i;
	if (dest < src)
	{
		for (i = 0; i < n; i++)
		{
			dest[i] = src[i];
		}
	}
	else if (dest > src)
	{
		for (i = n-1; i >= 0; i--)
		{
			dest[i] = src[i];
		}
	}
	return dest;
}
