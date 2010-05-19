#pragma once
#ifndef FORK_H
#define FORK_H

#include <stdint.h>
#include "memman.h"
#include "common.h"
#include "kheap.h"
#include "scheduler.h"

uint64_t fork_kernel();
uint64_t fork();

#endif