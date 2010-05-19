#include "keyboard.h"

static char* key = "##1234567890-=#\tQWERTYUIOP[]\n#ASDFGHJKL;\'`#\\ZXCVBNM,./#*# ##############789-456+123#######";
static int key_len;

static uint64_t occupant;
static uint64_t keyboard_lock;

static char scancode;
static char press_code;
static char break_code;
static int new_press;
static int new_break;

void keyboard_init()
{
	occupant = 0;
	new_press = 0;
	new_break = 0;
	key_len = strlen(key);
	keyboard_lock = register_lock();
}

void keyboard_handler(registers_t* regs)
{
	scancode = inb(0x60);
	if (scancode & 0x80)
	{
		new_break = 1;
		break_code = scancode;
	}
	else
	{
		new_press = 1;
		press_code = scancode;
	}
	if (occupant && new_press)
	{
		change_task_status(occupant, 0);
		asm volatile("int $0x20"); 
	}
}

char recognize_scancode(char scancode)
{
	if (scancode && scancode < key_len)
		return key[(uint8_t)scancode];
	return 0;
}

char get_char()
{
	if (!new_press)
	{
		change_task_status(occupant, 1);
		asm volatile("int $0x20");  
	}
	new_press = 0;
	return recognize_scancode(press_code);
}

char* readline(char* buffer)
{
	lock(keyboard_lock);
	occupant = get_lock_owner(keyboard_lock);
	new_press = 0;
	new_break = 0;

	int i = 0;
	char c;
	while ((c = get_char()) != '\n' && i < 79)
	{
		buffer[i] = c;
		putchar(c);
		i++;
	}
	if (buffer[i] != '\n')
		buffer[i++] = '\n';
	buffer[i] = '\0';
	putchar('\n');

	occupant = 0;
	unlock(keyboard_lock);
	return buffer;
}
