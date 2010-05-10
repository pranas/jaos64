
#include "keyboard.h"

#include "io.h"
#include "monitor.h"
#include "kheap.h"

char key[] = {
	0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
	'-', '=', 0,
	'\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
	0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
	0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
	'*', 0, ' ', 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0,
	'7', '8', '9', '-', 
	'4', '5', '6', '+',
	'1', '2', '3',
	0, 0,
	0, 0, 0,
	0, 0
};

void keyboard_handler(registers_t* regs)
{
	char scancode = inb(0x60);
}

char recognize_scancode(char scancode)
{
	if (scancode)
		return key[scancode];
	return 0;
}

char get_char()
{
	return 'a';
}

char* readline()
{
	char* buffer = kmalloc(81);
	int i = 0;
	char c;
	while ((c = get_char()) != '\n' && i < 79)
	{
		buffer[i++] = c;
	}
	if (buffer[i++] != '\n')
		buffer[i++] = '\n';
	buffer[i] = '\0';

	return buffer;
}
