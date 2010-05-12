
#include "syscall.h"

DEFN_SYSCALL1(puts, 0, const char *);
DEFN_SYSCALL1(puthex, 1, uint64_t);
DEFN_SYSCALL1(putint, 2, uint64_t);
DEFN_SYSCALL1(kmalloc, 3, uint64_t);
DEFN_SYSCALL1(kfree, 4, uint64_t);
DEFN_SYSCALL1(readline, 5, char *);
DEFN_SYSCALL0(fork, 6);
DEFN_SYSCALL1(exec, 7, char *);
DEFN_SYSCALL0(exit, 8);