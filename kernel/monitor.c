
#include <stdint.h>
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
int8_t cursor_x = 0;
int8_t cursor_y = 0;

/* video memory mapped here */
int16_t* video_memory = (int16_t*) 0xB8000;

// update cursor position
static void move_cursor()
{
	int16_t cursor_location = cursor_y * 80 + cursor_x;
	outb(VGA_INDEX_PORT, CURSOR_POSITION_HIGH);
	outb(VGA_DATA_PORT , cursor_location >> 8); // higher half
	outb(VGA_INDEX_PORT, CURSOR_POSITION_LOW);
	outb(VGA_DATA_PORT , cursor_location); // higher half
}

// scroll text
static void scroll()
{
	int8_t attribute_byte = ATTR_BYTE(BLACK, WHITE);
	int16_t blank = 0x20 | (attribute_byte << 8);

	if (cursor_y >= 25)
	{
		int i;
		for (i = 0; i < (80*24); i++)
			video_memory[i] = video_memory[i+80];
		for (i = 24*80; i < 25*80; i++)
			video_memory[i] = blank;

		cursor_y = 24;
	}
}

// write single char to screen
void putchar(char c)
{
	int8_t attribute_byte = ATTR_BYTE(BLACK, WHITE);
	int16_t* location;

	if (c == 0x08 && cursor_x) // backspace
	{
		cursor_x--;
	}
	else if (c == 0x09) // tab
	{
		cursor_x = (cursor_x + 8) & ~(8-1); // zero out 3 lowest bits
	}
	else if (c == '\r') // carry return
	{
		cursor_x = 0;
	}
	else if (c == '\n') // newline
	{
		cursor_x = 0;
		cursor_y++;
	}
	else if (c >= ' ') // printable chars
	{
		location = video_memory + (cursor_y * 80 + cursor_x);
		*location = c | (attribute_byte << 8); // attr. byte in higher half
		cursor_x++;
	}

	// check if a newline is necessary
	if (cursor_x >= 80)
	{
		cursor_x = 0;
		cursor_y++;
	}

	scroll();
	move_cursor();
}

void clear_screen()
{
	int16_t blank = 0x20 | (ATTR_BYTE(BLACK, WHITE) << 8);
	int i;
	for (i = 0; i < 80*25; i++)
	{
		video_memory[i] = blank;
	}

	cursor_x = 0;
	cursor_y = 0;
	move_cursor();
}

void puts(char* str)
{
    if (!str) return;
    
	int i = 0;
	while (str[i])
	{
		putchar(str[i++]);
	}
}

char tbuf[32];
char bchars[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

void itoa(uint64_t i, unsigned base, char* buf) {
   int pos = 0;
   int opos = 0;
   int top = 0;

   if (i == 0 || base > 16) {
      buf[0] = '0';
      buf[1] = '\0';
      return;
   }

   while (i != 0) {
      tbuf[pos] = bchars[i % base];
      pos++;
      i /= base;
   }
   top=pos--;
   for (opos=0; opos<top; pos--,opos++) {
      buf[opos] = tbuf[pos];
   }
   buf[opos] = 0;
}

void itoa_s(int64_t i, unsigned base, char* buf) {
   if (base > 16) return;
   if (i < 0) {
      *buf++ = '-';
      i *= -1;
   }
   itoa(i,base,buf);
}

void uitoa_s(uint64_t i, unsigned base, char* buf) {
   if (base > 16) return;
   itoa(i,base,buf);
}

void putint(int64_t i)
{
	char str[32]={0};
	itoa_s (i, 10, str);
	puts (str);
}

void puthex(uint64_t i)
{
    char str[32]={0};
    str[0] = '0';
    str[1] = 'x';
	uitoa_s (i, 16, str+2);
	puts (str);
}
