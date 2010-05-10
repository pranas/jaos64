#pragma once
#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "isr.h"
#include "io.h"
#include "monitor.h"

void keyboard_handler(registers_t*);

#endif
