#pragma once
#ifndef SYSCALL_H
#define SYSCALL_H

#include "isr.h"
#include "monitor.h"
#include "idt.h"
#include "kheap.h"
#include "keyboard.h"
#include "fork.h"

void syscalls_init();
void syscall_handler(registers_t * regs);

#endif

