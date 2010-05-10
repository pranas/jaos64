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

uint64_t register_lock()
{
    locks[next_lockid].head = 0;
    locks[next_lockid].tail = 0;
    return next_lockid++;
}

void lock(int lockid)
{
    cli();
    
    // unlocked
    spin_lock(&master_lock);
    if (locks[lockid].head == 0)
    {
        // puts("No one in line, going to lock and return :)\n");
        struct lock* lck = kmalloc(sizeof(struct lock));
        lck->pid = get_current_pid();
        lck->next = 0;
        locks[lockid].head = lck;
        locks[lockid].tail = lck;
        master_lock = 0;
        sti();
        return;
    }
    
    // puts("Need to stand in line.. First PID:");
    // putint(locks[lockid].head->pid);
    // puts("\n");
    
    // some one is in locked zone
    // lets take place in queue
    
    struct lock* lck = kmalloc(sizeof(struct lock));
    lck->pid = get_current_pid();
    lck->next = 0;
    locks[lockid].tail->next = lck;
    locks[lockid].tail = lck;
    master_lock = 0;
    
    // wait in line
    for(;;)
    {
        spin_lock(&master_lock);
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

void unlock(int lockid)
{
    cli();
    // only needed for SMP, cause cli(), sti() protects from other processes on same cpu
    spin_lock(&master_lock);
    
    struct lock* tmp;
    tmp = locks[lockid].head;
    
    // if we have 1 element in queue
    if (locks[lockid].head == locks[lockid].tail)
    {
        locks[lockid].head = 0;
        locks[lockid].tail = 0;
        kfree(tmp);
        sti();
        return;
    }
    
    // if there is more than 1
    locks[lockid].head = tmp->next;
    kfree(tmp);
    //unblock locks[lockid]->head->pid
    change_task_status(locks[lockid].head->pid, 0);
    master_lock = 0; // spin unlock
    sti();
    return;    
}