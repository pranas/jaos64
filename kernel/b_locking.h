#pragma once
#ifndef B_LOCKING_H
#define B_LOCKING_H

/*

	b_locking.h
	
	(B)Locking system

	Copyright (C) 2010

	This is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2, or (at your option)
	any later version.

*/

#include <stdint.h>
#include "common.h"
#include "scheduler.h"
#include "kheap.h"

#define LOCKING_MAX 0x10

extern void spin_lock(uint64_t* var);

struct lock
{
    uint64_t pid;
    struct lock* next;
} __attribute__((packed));

struct lock_queue
{
    struct lock* head;
    struct lock* tail;
} __attribute__((packed));

typedef struct lock_queue lock_queue;

uint64_t register_lock();
void lock(int lockid);
void unlock(int lockid);

uint64_t get_lock_owner(int lockid);

#endif