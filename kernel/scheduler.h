#ifndef SCHEDULER_H
#define SCHEDULER_H

/*

	scheduler.h
	
	Scheduler
	
	Copyright (C) 2010

	This is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2, or (at your option)
	any later version.

*/

#include <stdint.h>
#include "memman.h"
#include "isr.h"

struct task
{
   uint64_t pid;           // Process ID.
   uint64_t rsp, rbp;     // Stack and base pointers.
   uint64_t rip;          // Instruction pointer.
   pml4_entry* pml4; 	  // Page directory.
};

typedef struct task task;

static task* current_task = 0;
static task task_list[32];
static uint64_t next_pid = 0;
static uint64_t ret = 0;

extern uint64_t read_rip();

void scheduler_init();
uint64_t fork_kernel();
uint64_t fork();
uint64_t getpid();
void switch_task();

#endif