#pragma once
#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>

#define DECL_SYSCALL0(fn) uint64_t syscall_##fn();
#define DECL_SYSCALL1(fn,p1) uint64_t syscall_##fn(p1);

#define DEFN_SYSCALL0(fn, num) \
uint64_t syscall_##fn() \
{ \
 uint64_t a; \
 asm volatile("int $0x80" : "=a" (a) : "0" (num)); \
 return a; \
}
// asm ("xchg %bx, %bx"); \

#define DEFN_SYSCALL1(fn, num, P1) \
uint64_t syscall_##fn(P1 p1) \
{ \
 uint64_t a; \
 asm volatile("movq %2, %%rdi\n\t" \
		 "int $0x80" \
		 : "=a" (a) \
		 : "0" (num), "m" (p1)); \
 return a; \
}

DECL_SYSCALL1(puts, const char *);
DECL_SYSCALL1(puthex, uint64_t);
DECL_SYSCALL1(putint, uint64_t);
DECL_SYSCALL1(kmalloc, uint64_t);
DECL_SYSCALL1(kfree, uint64_t);
DECL_SYSCALL1(readline, char *);
DECL_SYSCALL0(fork);
DECL_SYSCALL1(exec, char *);
DECL_SYSCALL0(exit);
DECL_SYSCALL0(sleep);

#endif
