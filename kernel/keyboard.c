
#include "keyboard.h"

#include "io.h"
#include "monitor.h"

void keyboard_handler()
{
	char scancode = inb(0x60);
	puthex(scancode); puts("\n");
}
