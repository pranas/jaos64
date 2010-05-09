#include "fork.h"

uint64_t fork_kernel()
{
	// kernel's child will start with brand new stack
	// this hack finds where it should return
	asm volatile("mov 0x8(%%rbp), %0" : "=r"(kernel_fork_ret));
	
    cli();
	
	task* parent_task = current_task;
    task* new_task = kmalloc(sizeof(task));
	
	new_task->pid = next_pid++;
	// allocate stack in user space
	new_task->rbp = new_task->rsp = alloc_page(0xbffff000, 1) + 0x1000;
	new_task->rip = 0;
	new_task->pml4 = clone_pml4t();
    new_task->next = 0;
    
    add_task(new_task);
	
	// entry point for child
	uint64_t rip = read_rip();
	
    if (parent_task == current_task)
	{
		// parents should take care of their children
		// (setting up stuff for child)
		new_task->rip = rip;
        sti();
		return new_task->pid;
	}
	else
	{
		// child should return with 0 (using hacked return address)
		asm volatile("         \ 
		     mov %0, %%rcx;       \ 
		     mov $0x0, %%rax; \ 
		     jmp *%%rcx           "
		                : : "r"(kernel_fork_ret));
	}
	
}