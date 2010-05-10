
#ifndef _SYSCALL_H
#define _SYSCALL_H

#include "isr.h"

#define DECL_SYSCALL0(fn) int syscall_##fn();
#define DECL_SYSCALL1(fn,p1) int syscall_##fn(p1);

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

DECL_SYSCALL1(puts, const char *);
DECL_SYSCALL1(puthex, int64_t);
DECL_SYSCALL1(putint, int64_t);

void syscalls_init();
void syscall_handler(registers_t * regs);

#endif

