#include "syscall.h"

#include "isr.h"
#include "monitor.h"

void syscall_handler(registers_t *regs);

const int syscall_num = 3;
void * syscalls[syscall_num] =
{
   &puts,
   &puthex,
   &putint,
};

void init_syscalls()
{
   register_interrupt_handler (0x80, &syscall_handler);
}

void syscall_handler(registers_t regs)
{
   if (regs->rax >= syscall_num)
       return;
   void *location = syscalls[regs->rax];

   int ret;
   asm volatile ("call *%0\n\t"
	 : "=a" (ret)
	 : "r" (location));
   regs->rax = ret;
} 

