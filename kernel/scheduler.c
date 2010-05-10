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
    cli();
	puts("Initializing scheduler...\n");
   
	// init kernel task
    current_task = task_list = (task*) kmalloc(sizeof(task));
	
	current_task->pid = next_pid++;
	current_task->rbp = 0;
	current_task->rsp = 0;
	current_task->rip = 0;
	current_task->pml4 = get_current_pml4();
    current_task->next = 0;
	
    register_handler(0x20, switch_task);
    sti();
}

void add_task(task* new_task)
{
    new_task->pid = next_pid++;
    task* tmp = task_list;
    while(tmp->next)
    {
        tmp = tmp->next;
    }
    tmp->next = new_task;
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
    
    // round robin untill unblocked task will be found
    do
    {
        if (!current_task->next) current_task = task_list;
        else current_task = current_task->next;
    } while (current_task->status != 0);

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

uint64_t get_current_pid()
{
    return current_task->pid;
}

task* get_current_task()
{
    return current_task;
}

void change_task_status(uint64_t pid, uint8_t status)
{
    task* tmp = task_list;
    while(tmp->next)
    {
        if (tmp->pid = pid)
        {
            tmp->status = status;
        }
        else
        {
            tmp = tmp->next;
        }
    }
    tmp->next = new_task;
}

void debug_task(task* tsk)
{
    puts("PID: ");
    puthex(tsk->pid);
    puts(" RIP: ");
    puthex(tsk->rip);
    puts(" PML4: ");
    puthex(tsk->pml4);
    puts(" Next: ");
    puthex(tsk->next);
    puts("\n");
}