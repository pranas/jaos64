#include "syscall.h"

#include "isr.h"
#include "monitor.h"
#include "idt.h"

void syscall_handler(registers_t *regs);

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
   asm volatile ("call *%1\n\t"
	 : "=a" (ret)
	 : "r" (location));
   regs->rax = ret;
} 

