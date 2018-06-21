#include <ctype.h>
#include "textgfx.h"
#include "../lang.h"

void putch_ascii(int ch)
{
	switch (ch) {
	case HLINE:
		ch = '-';
		break;
	case VLINE:
		ch = '|';
		break;
	case TEXTURE1:
		ch = '#';
		break;
	case TEXTURE2:
	case ' '|0x100:
		ch = ' ';
		break;
	case BULLET:
		ch = '.';
		break;
	case UPARROW:
		ch = '^';
		break;
	default:
		ch = '+';
	}
	putch(ch);
}

int putnchars(int ch, int n)
{
	int i = 0;
	for (; i < n; i++)
		putch(ch);
	return n;
}

void printstr_acs(const char *str, int n)
{
	int ch = 0;
	while (*str) {
		if (islower(*str) || (ch & 0x100) && *str == ' ')
			ch = *str | 0x100;
		else
			ch = *str;
		putch(ch);
		str++;
		if (*str == 'N') {
			putnchars(ch, n-1);
			str++;
		}
	}
}

int isprintable(int c)
{
	return c >= ' ' && c < 0x7F ||
	       lang & LATIN1 && (unsigned char) c >= 0xA0;
}
