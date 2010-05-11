#pragma once
#ifndef COMMON_H
#define COMMON_H

#define sti() asm volatile ("sti");
#define cli() asm volatile ("cli");
#define nop() asm volatile ("nop");
#define magicbp() asm ("xchg %bx, %bx");

#endif
