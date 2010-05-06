/*

	scheduler.c

	Scheduler
	
	Copyright (C) 2010

	This is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2, or (at your option)
	any later version.

*/

#include "scheduler.h"

void scheduler_init()
{
	asm volatile("cli");
	
	puts("Initializing scheduler...\n");
   
	// init kernel task
	current_task = &task_list[next_pid];
	
	current_task->pid = next_pid++;
	current_task->rbp = 0;
	current_task->rsp = 0;
	current_task->rip = 0;
	current_task->pml4 = get_current_pml4();
	
	register_handler(0x20, switch_task);

   	asm volatile("sti");
}

uint64_t fork_kernel()
{
	// kernel's child will start with brand new stack
	// this hack finds where it should return
	asm volatile("mov 0x8(%%rbp), %0" : "=r"(ret));
	
	asm volatile("cli");
	
	task* parent_task = current_task;
	task* new_task = &task_list[next_pid];
	
	new_task->pid = next_pid++;
	new_task->rbp = new_task->rsp = alloc_page(0xbffff000, 1) + 0x1000;
	new_task->rip = 0;
	new_task->pml4 = clone_pml4t();
	
	// entry point for child
	uint64_t rip = read_rip();
	
    if (parent_task == current_task)
	{
		// parents should take care of their children
		// (setting up stuff for child)
		new_task->rip = rip;
		asm volatile("sti");
		return new_task->pid;
	}
	else
	{
		// child should return with 0 (using hacked return address)
		asm volatile("         \ 
		     mov %0, %%rcx;       \ 
		     mov $0x0, %%rax; \ 
		     jmp *%%rcx           "
		                : : "r"(ret));
	}
	
}

void switch_task()
{
	// if scheduling is not initialized
	if (!current_task)
	{
       	return;
	}

	uint64_t rsp, rbp, rip;
	asm volatile("mov %%rsp, %0" : "=r"(rsp));
	asm volatile("mov %%rbp, %0" : "=r"(rbp));
	
	rip = read_rip();
	
	if (rip == 0x12345)
	{
       	return;
	}
	
	current_task->rip = rip;
   	current_task->rsp = rsp;
   	current_task->rbp = rbp;

	if (current_task->pid < (next_pid - 1))
	{
		current_task = &task_list[current_task->pid+1];
	}
	else
	{
		current_task = &task_list[0];
	}
	
	rsp = current_task->rsp;
   	rbp = current_task->rbp;
	rip = current_task->rip;

	asm volatile("         \ 
	     cli;                 \ 
	     mov %0, %%rcx;       \ 
	     mov %1, %%rsp;       \ 
	     mov %2, %%rbp;       \ 
	     mov %3, %%cr3;       \ 
	     mov $0x12345, %%rax; \ 
	     sti;                 \ 
	     jmp *%%rcx           "
	                : : "r"(rip), "r"(rsp), "r"(rbp), "r"(current_task->pml4));
	
	
}