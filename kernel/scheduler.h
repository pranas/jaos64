#pragma once
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

struct task
{
   uint64_t pid;           // Process ID.
   uint64_t rsp, rbp;     // Stack and base pointers.
   uint64_t rip;          // Instruction pointer.
   struct pml4_entry* pml4; 	  // Page directory.
   uint8_t status;          // 0 - free, 1 - blocked
   struct task* next;
}__attribute__((packed));

typedef struct task task;

extern uint64_t read_rip();

void scheduler_init();
void switch_task();
void add_task(task* new_task);
void change_task_status(uint64_t pid, uint8_t status);
uint64_t get_current_pid();
task* get_current_task();
void debug_task(task* tsk);

#endif