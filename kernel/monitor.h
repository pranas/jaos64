#pragma once
#ifndef MONITOR_H
#define MONITOR_H

#include <stdint.h>
#include "b_locking.h"
#include "io.h"

/* We use the index port to specify what we want to do.
 * We do the actual read/writes with the data port. */

#define VGA_INDEX_PORT 0x3D4
#define VGA_DATA_PORT 0x3D5

/* Since 80x25=2000, position cant fit into 8 bytes,
 * so there are 2 registers for position information */
#define CURSOR_POSITION_HIGH 0xE
#define CURSOR_POSITION_LOW 0xF

/* Color helpe macros */
#define BLACK 0
#define WHITE 15
#define ATTR_BYTE(bg,fg) (((bg) << 4) | ((fg) & 0x0F))

/* initial position is top left */
static int8_t cursor_x = 0;
static int8_t cursor_y = 0;

/* video memory mapped here */
static int16_t* video_memory = (int16_t*) 0xB8000; // default

static uint64_t monitor_lock = -1;

void monitor_init();

void set_video_memory(void* address);
void* get_video_memory();

void clear_screen ();
void putchar (char c);
void puts (char* str);
void putint (int64_t);
void puthex (uint64_t);

#endif
