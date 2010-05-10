
#include "keyboard.h"

#include "io.h"
#include "monitor.h"
#include "kheap.h"
#include "b_locking.h"
#include <string.h>

static char* key = "##1234567890-=#\tqwertyuiop[]\n#asdfghjkl;\'`#\\zxcvbnm,./#*# ##############789-456+123#######";

static uint64_t occupant;
static uint64_t keyboard_lock;

static char scancode;
static int new_scancode;

void keyboard_init()
{
	occupant = 0;
	new_scancode = 0;
	keyboard_lock = register_lock();
	puts("keyboard lock is "); putint(keyboard_lock); puts("\n");
}

void keyboard_handler(registers_t* regs)
{
	scancode = inb(0x60);
	new_scancode = 1;
	if (occupant)
	{
		puts("waking up the occupant\n");
		change_task_status(occupant, 0);
	}
}

char recognize_scancode(char scancode)
{
	if (scancode && scancode < strlen(key))
		return key[scancode];
	return 0;
}

char get_char()
{
	puts("get_char start\n");
	if (!new_scancode)
	{
		change_task_status(occupant, 1);
	}
	new_scancode = 0;
	puts("get_char returning\n");
	return recognize_scancode(scancode);
}

char* readline()
{
	lock(keyboard_lock);
	occupant = get_lock_owner(keyboard_lock);
	puts("The occupant's PID is "); putint(occupant); puts("\n");

	char* buffer = kmalloc(81);
	puthex(buffer); puts("\n");
	int i = 0;
	char c;
	puts("looping on get_char\n");
	while ((c = get_char()) != '\n' && i < 79)
	{
		buffer[i] = c;
		putchar(c);
		i++;
	}
	puts("out of loop\n");
	if (buffer[i] != '\n')
		buffer[i++] = '\n';
	buffer[i] = '\0';
	putchar("\n");

	occupant = 0;
	unlock(keyboard_lock);
	puts("readline done\n");
	return buffer;
}
