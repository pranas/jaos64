#include <stdint.h>

void set_video_memory(void* address);
void* get_video_memory();

void clear_screen ();
void putchar (char c);
void puts (char* str);
void putint (int64_t);
void puthex (uint64_t);
