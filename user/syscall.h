#pragma once
#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>

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
DECL_SYSCALL1(puthex, uint64_t);
DECL_SYSCALL1(putint, uint64_t);
DECL_SYSCALL1(kmalloc, uint64_t);
DECL_SYSCALL1(kfree, uint64_t);
DECL_SYSCALL0(readline);

/*int syscall_puts(const char *);
int syscall_puthex(uint64_t);
int syscall_putint(uint64_t);
int syscall_kmalloc(uint64_t);
int syscall_kfree(uint64_t);
int syscall_readline();*/

#endif