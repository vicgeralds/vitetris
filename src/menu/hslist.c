#include <stdlib.h>
#include "menu.h"
#include "../input/input.h"
#include "../textgfx/textgfx.h"
#include "../hiscore.h"

void show_hiscorelist(int x, int y)
{
	char buf[320];
	char *p;
	int n;
	setcurs(x, y);
	printmenuitem("Highscores", 1); newln(x);
	setcolorpair(MAGENTA_FG);
	printstr("    Name      Score  Lvl  Lines"); newln(x);
	setattr_normal();
	if (!readhiscores(NULL)) {
		printstr("    No saved scores");
		newln(x);
	} else {
		n = gethiscorelist(buf);
		p = buf;
		while (n) {
			p[30] = '\0';
			printstr(p);
			newln(x);
			p += 32;
			n--;
		}
	}
	refreshwin(-1);
	if (getkeypress_block(SINGLE_PL) == 'q')
		exit(0);
}
