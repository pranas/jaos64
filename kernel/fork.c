#include "fork.h"

static uint64_t kernel_fork_ret = 0;

uint64_t fork_kernel()
{
	// kernel's child will start with brand new stack
	// this hack finds where it should return
	asm volatile("mov 0x8(%%rbp), %0" : "=r"(kernel_fork_ret));
	
    cli();
	
	task* parent_task = (task*) get_current_task();
    task* new_task = (task*) kmalloc(sizeof(task));
	
	// allocate stack in user space
	new_task->rbp = new_task->rsp = (uint64_t) alloc_page((void *) 0xbffff000, 1) + 0x1000;
	new_task->rip = 0;
	new_task->pml4 = clone_pml4t();
    new_task->next = 0;
    
    // adds to list and sets pid
    add_task(new_task);
	
	// entry point for child
	uint64_t rip = read_rip();
	
    //if (parent_task == current_task)
    if(parent_task == get_current_task())
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
    return 0; // never happens
	
}

// fork for user applications
uint64_t fork()
{
    cli();
    
	task* parent_task = (task*) get_current_task();
    task* new_task = (task*) kmalloc(sizeof(task));
	
    new_task->rbp = new_task->rsp = 0;
	new_task->rip = 0;
	new_task->pml4 = clone_pml4t();
    new_task->next = 0;
    
    // adds to list and sets pid
    add_task(new_task);
	
	// entry point for child
	uint64_t rip = read_rip();
	
    if (parent_task == get_current_task())
	{
		// parents should take care of their children
		// (setting up stuff for child)
		uint64_t rsp, rbp; 
		asm volatile("mov %%rsp, %0" : "=r"(rsp));
        asm volatile("mov %%rbp, %0" : "=r"(rbp));
        new_task->rsp = rsp;
        new_task->rbp = rbp;
		new_task->rip = rip;
        sti();
		return new_task->pid;
	}
	else
	{
        return 0;
	}
	
}