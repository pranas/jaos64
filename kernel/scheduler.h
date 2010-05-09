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
#include "common.h"
#include "memman.h"
#include "kheap.h"
#include "fork.h"
#include "isr.h"

struct task
{
   uint64_t pid;           // Process ID.
   uint64_t rsp, rbp;     // Stack and base pointers.
   uint64_t rip;          // Instruction pointer.
   pml4_entry* pml4; 	  // Page directory.
   struct task* next;
}__attribute__((packed));

typedef struct task task;

static volatile task* current_task = 0;
static volatile task* task_list;
static uint64_t next_pid = 0;

extern uint64_t read_rip();

void scheduler_init();
void switch_task();
void add_task(task* new_task);
uint64_t getpid();

#endif