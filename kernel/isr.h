#pragma once
#ifndef ISR_H
#define ISR_H

#include <stdint.h>

typedef struct registers
{
	int64_t ds;
	int64_t r9, r8, rsp, rbp, rdi, rsi, rdx, rcx, rbx, rax; // we need to preserve these
	int64_t int_no, err_code;
	int64_t rip, cs, rflags, userrsp, ss;
} registers_t;


#include "monitor.h"
#include "apic.h"

#define MAX_HANDLERS 255

typedef void (*isr_t) (registers_t*);
void register_handler(int int_no, isr_t custom_handler);

void isr_handler(registers_t regs);

#endif