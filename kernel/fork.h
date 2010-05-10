#pragma once
#ifndef FORK_H
#define FORK_H

#include <stdint.h>
#include "memman.h"
#include "common.h"
#include "kheap.h"
#include "scheduler.h"

static uint64_t kernel_fork_ret = 0;

uint64_t fork_kernel();
uint64_t fork();

#endif