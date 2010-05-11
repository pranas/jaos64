/*

	b_locking.c
	
	(B)Locking system

	Copyright (C) 2010

	This is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2, or (at your option)
	any later version.

*/

#include "b_locking.h"

static volatile lock_queue locks[LOCKING_MAX];
static volatile uint64_t master_lock = 0;
static volatile uint64_t next_lockid = 0;

uint64_t register_lock()
{
    locks[next_lockid].head = 0;
    locks[next_lockid].tail = 0;
    return next_lockid++;
}

void lock(int lockid)
{
    if (lockid == -1) return;
    cli();
    
    // unlocked
    spin_lock((uint64_t*) &master_lock);
    if (locks[lockid].head == 0)
    {
        struct lock* lck = (struct lock*) kmalloc(sizeof(struct lock));
        lck->pid = get_current_pid();
        lck->next = 0;
        locks[lockid].head = lck;
        locks[lockid].tail = lck;
        master_lock = 0;
        sti();
        return;
    }
    
    // if lock is aquired by this same process
    // this could happen if let's say `puts` would call `putchar` wich is (should be)
    // protected by the same lock
    if (locks[lockid].head->pid == get_current_pid())
    {
        master_lock = 0;
        sti();
        return;
    }
    
    // some one is in locked zone
    // lets take place in queue
    
    struct lock* lck = (struct lock*) kmalloc(sizeof(struct lock));
    lck->pid = get_current_pid();
    lck->next = 0;
    locks[lockid].tail->next = lck;
    locks[lockid].tail = lck;
    master_lock = 0;
    
    // wait in line
    for(;;)
    {
        spin_lock((uint64_t*) &master_lock);
        if (locks[lockid].head->pid == get_current_pid())
        {
            // it's our time!
            master_lock = 0;
            sti();
            return;
        }
        // puts("Not my turn, voluntarily blocking...\n");
        master_lock = 0;
        sti();
        // block process
        change_task_status(get_current_pid(), 1);
        // voluntarily switch
        asm volatile("int $0x20");    
    }
}

uint64_t get_lock_owner(int lockid)
{
    return locks[lockid].head->pid;
}

void unlock(int lockid)
{
    if (lockid == -1) return;
    
    cli();
    // only needed for SMP, cause cli(), sti() protects from other processes on same cpu
    spin_lock((uint64_t*) &master_lock);
    
    struct lock* tmp;
    tmp = locks[lockid].head;
    
    if (!tmp)
    {
        // means it's not locked, we don't need to unlock anything
        // this may happen if we unlocked somewhere deeper in stack and now we
        // are returning
        master_lock = 0;
        kfree(tmp);
        sti();
        return;
    }
    
    // if we have 1 element in queue
    if (locks[lockid].head == locks[lockid].tail)
    {
        locks[lockid].head = 0;
        locks[lockid].tail = 0;
        master_lock = 0;
        kfree(tmp);
        sti();
        return;
    }
    
    // if there is more than 1
    // only remove lock if it belongs to us
    if (locks[lockid].head->pid == get_current_pid())
    {
        locks[lockid].head = tmp->next;
        kfree(tmp);
    }
    //unblock locks[lockid]->head->pid
    change_task_status(locks[lockid].head->pid, 0);
    master_lock = 0; // spin unlock
    sti();
    return;    
}
