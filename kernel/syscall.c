#include "syscall.h"

#include "isr.h"
#include "monitor.h"
#include "idt.h"

DEFN_SYSCALL1(puts, 0, const char *);
DEFN_SYSCALL1(puthex, 1, int64_t);
DEFN_SYSCALL1(putint, 2, int64_t);

#define SYSCALL_NUM 3
void * syscalls[SYSCALL_NUM] =
{
   &puts,
   &puthex,
   &putint,
};

void init_syscalls()
{
   register_handler (0x80, &syscall_handler);
}

void syscall_handler(registers_t* regs)
{
   if (regs->rax >= SYSCALL_NUM)
       return;
   void *location = syscalls[regs->rax];
   int ret;
   asm volatile ("xchg %%bx, %%bx\n\t"
		   "pushq %1\n\t"
		   "pushq %2\n\t"
		   "pushq %3\n\t"
		   "pushq %4\n\t"
		   "pushq %5\n\t"
		   "pushq %6\n\t"
		   "pushq %7\n\t"
		   "pop %%r11\n\t"
		   "pop %%r9\n\t"
		   "pop %%r8\n\t"
		   "pop %%rcx\n\t"
		   "pop %%rdx\n\t"
		   "pop %%rsi\n\t"
		   "pop %%rdi\n\t"
		   "call %%r11\n\t"
	 : "=a" (ret)
	 : "m" (regs->rdi), "m" (regs->rsi), "m" (regs->rdx),
	   "m" (regs->rcx), "m" (regs->r8) , "m" (regs->r9), "m" (location));
   regs->rax = ret;
} 

