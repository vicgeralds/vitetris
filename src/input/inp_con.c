#include <conio.h>
#include <stdio.h>	/* sprintf */
#include "termin.h"
#include "keyboard.h"
#include "input.h"

void set_input_mode() {}
void restore_input_mode() {}
void init_keybd() {}

int kb_readkey(unsigned char *dest)
{
	int c;
	if (!kbhit())
		return 0;
	c = getch();
	if (!c || c==0xE0) {
		c = getch();
		switch (c) {
		case 75: c = MVLEFT;  break;
		case 77: c = MVRIGHT; break;
		case 72: c = MVUP;    break;
		case 80: c = MVDOWN;  break;
		case 83: c = DEL;     break;
		default:
			 dest[0] = '\0';
			 dest[1] = c;
			 return 2;
		}
	} else if (c =='\r')
		c ='\n';
	*dest = c;
	return 1;
}

int kb_toascii(const unsigned char *key)
{
	return ESC+1;
}

void kb_flushinp()
{
	while (kbhit())
		getch();
}

const char *kb_keyname(unsigned char *key, int n)
{
	if (n < 2)
		return NULL;
	sprintf(key, "0+%X", key[1]);
	return key;
}

int inpselect_dev(int tm)
{
	return 0;
}
