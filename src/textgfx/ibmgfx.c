#include <string.h>

int ibmgfx(int ch)
{
	const char *vt100 = "lmkjnqxtuvwah~-";
	const char *cp437 = "\xDA\xC0\xBF\xD9\xC5\xC4\xB3"
			    "\xC3\xB4\xC1\xC2\xB1\xB0\xFA\x18";
	char *p;
	if (ch & 0x100) {
		ch ^= 0x100;
		p = strchr(vt100, ch);
		if (p)
			return cp437[p-vt100];
	}
	return ch;
}
