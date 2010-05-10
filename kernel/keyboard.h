#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include "isr.h"

void keyboard_handler(registers_t*);

enum SCANCODE {
	escape = 0x1,
	num_1, num_2, num_3, num_4, num_5, num_6, num_7, num_8, num_9, num_0,
	minus, equals, backspace,
	tab, q, w, e, r, t, y, u, i, o, p, left_bracket, right_bracket, enter,
	lctrl, a, s, d, f, g, h, j, k, l, semicolon, quote, backquote,
	lshift, backslash, z, x, c, v, b, n, m, comma, period, slash, rshift,
	keypad_mult, lalt, space, caps_lock,
	f1, f2, f3, f4, f5, f6, f7, f8, f9, f10,
	num_lock, scroll_lock,
	keypad_7, keypad_8, keypad_9, keypad_minus,
	keypad_4, keypad_5, keypad_6, keypad_plus,
	keypad_1, keypad_2, keypad_3,
	keypad_ins, keypad_del,
	alt_sysrq, f11_rare, win,
	f11, f12
};

char recognize_scancode(char scancode);
char* readline();
void keyboard_init();
char get_char();

#endif
