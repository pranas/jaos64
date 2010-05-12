#pragma once
#ifndef EXEC_H
#define EXEC_H

#include <stdlib.h>
#include "memman.h"
#include "kheap.h"
#include "elf.h"

extern void switch_to_user_mode(void*);

uint64_t exec(char* filename);

#endif