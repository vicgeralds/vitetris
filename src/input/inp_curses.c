#include <curses.h>
#include "termin.h"
#include "keyboard.h"
#include "input.h"

void set_input_mode()
{
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	nodelay(stdscr, TRUE);
#ifdef PDCURSES
	PDC_return_key_modifiers(TRUE);
#endif
}

void init_keybd() {}

int kb_readkey(unsigned char *dest)
{
	const short funckeys[7] = {
		KEY_LEFT, KEY_RIGHT, KEY_UP, KEY_DOWN,
		KEY_ENTER, KEY_BACKSPACE, KEY_DC
	};
	const unsigned char keys[7] = {
		MVLEFT, MVRIGHT, MVUP, MVDOWN,
		STARTBTN, BACKSPACE, DEL
	};
	int c = getch();
	int i;
	if (c == ERR)
		return 0;

#if PDCURSES && (WIN32 || __WIN32__)
	/* PDCurses 3.3 in Win 98 workarounds. */
	/* I get 0xCA|character every now and then */
	if (c > KEY_MAX)
		c &= 0xff;
	/* I get right shift after arrow keys all the time */
	if (c == KEY_SHIFT_R)
		return 0;
#endif
#ifdef KEY_RESIZE
	if (c == KEY_RESIZE)
		return 0;
#endif
	for (i=0; i<7; i++)
		if (c == funckeys[i]) {
			dest[0] = keys[i];
			return 1;
		}
#ifdef PADENTER
	if (c == PADENTER) {
		dest[0] = STARTBTN;
		return 1;
	}
#endif
	if (c >= 0x100) {
		dest[0] = c & 0xFF;
		dest[1] = c >> 8;
		return 2;
	}
	dest[0] = c;
	return 1;
}

int kb_toascii(const unsigned char *key)
{
	short numpad[10] = {
#ifdef PDCURSES
		PAD0, KEY_C1, KEY_C2, KEY_C3,
		      KEY_B1, KEY_B2, KEY_B3,
	              KEY_A1, KEY_A2, KEY_A3
#else
		0, KEY_C1, 0, KEY_C3, 0, KEY_B2, 0, KEY_A1, 0, KEY_A3
#endif
	};
	int ch = key[0] | key[1] << 8;
	int i;
	for (i=0; i < 10; i++)
		if (numpad[i] == ch)
			return i+'0';
	return ESC+1;
}

void kb_flushinp()
{
	flushinp();
}

const char *kb_keyname(unsigned char *key, int n)
{
	unsigned c;
	if (n < 2)
		return NULL;
	c = key[0];
	c |= key[1] << 8;
#ifdef PDCURSES
	switch (c) {
	case KEY_SHIFT_L:   return "LSHFT";
	case KEY_SHIFT_R:   return "RSHFT";
	case KEY_CONTROL_L: return "LCTRL";
	case KEY_CONTROL_R: return "RCTRL";
	case KEY_ALT_L:     return "ALT";
	case KEY_ALT_R:     return "ALTGR";
	}
#endif
	sprintf(key, "%X", c);
	return key;
}

int inpselect_dev(int tm)
{
	return 0;
}
