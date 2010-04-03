#ifndef _ISR_H
#define _ISR_H

#include <stdint.h>

typedef struct registers
{
	int64_t ds;
	int64_t rsp, rbp, rdi, rsi, rdx, rcx, rbx, rax; // we need to preserve these
	int64_t int_no, err_code;
	int64_t rip, cs, rflags, userrsp, ss;
} registers_t;

void isr_handler(registers_t regs);

#endif
