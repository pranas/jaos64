
#include "syscall.h"

#define DEFN_SYSCALL0(fn, num) \
int syscall_##fn() \
{ \
 int a; \
 asm volatile("int $0x80" : "=a" (a) : "0" (num)); \
 return a; \
}

#define DEFN_SYSCALL1(fn, num, P1) \
int syscall_##fn(P1 p1) \
{ \
 int a; \
 asm volatile("movq %2, %%rdi\n\t" \
		 "int $0x80" \
		 : "=a" (a) \
		 : "0" (num), "m" (p1)); \
 return a; \
}

DEFN_SYSCALL1(puts, 0, const char *);
DEFN_SYSCALL1(puthex, 1, uint64_t);
DEFN_SYSCALL1(putint, 2, uint64_t);
DEFN_SYSCALL1(kmalloc, 3, uint64_t);
DEFN_SYSCALL1(kfree, 4, uint64_t);
DEFN_SYSCALL0(readline, 5);

/*int syscall_puts(const char * p1)
{
	int num = 0;
	int a;
	asm volatile("movq %2, %%rdi\n\t"
		"int $0x80"
		: "=a" (a)
		: "0" (num), "m" (p1));
	return a;
}

int syscall_puthex(uint64_t p1)
{
	int num = 1;
	int a;
	asm volatile("movq %2, %%rdi\n\t"
		"int $0x80"
		: "=a" (a)
		: "0" (num), "m" (p1));
	return a;
}

int syscall_putint(uint64_t p1)
{
	int num = 2;
	int a;
	asm volatile("movq %2, %%rdi\n\t"
		"int $0x80"
		: "=a" (a)
		: "0" (num), "m" (p1));
	return a;
}

int syscall_kmalloc(uint64_t p1)
{
	int num = 3;
	int a;
	asm volatile("movq %2, %%rdi\n\t"
		"int $0x80"
		: "=a" (a)
		: "0" (num), "m" (p1));
	return a;
}

int syscall_kfree(uint64_t p1)
{
	int num = 4;
	int a;
	asm volatile("movq %2, %%rdi\n\t"
		"int $0x80"
		: "=a" (a)
		: "0" (num), "m" (p1));
	return a;
}

int syscall_readline()
{
	int num = 5;
	int a;
	asm volatile("int $0x80" : "=a" (a) : "0" (num));
	return a;
} */
