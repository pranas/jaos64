#include <stdlib.h>

void* memcpy(void *dest, const void *src, size_t n)
{
	int i;
	for (i = 0; i < n; i++)
	{
		dest[i] = src[i];
	}
	return dest;
}

void* memmove(void *dest, const void *src, size_t n)
{
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
