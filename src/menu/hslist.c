/* Show highscore lists */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
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
	printmenuitem("Highscores", 1);
	movefwd(1);
	printstr(getmodestr());
	newln(x);
	setcolorpair(MAGENTA_FG);
	readhiscores(NULL);
	printstr(hiscore_columns); newln(x);
	setattr_normal();
	if (!num_hiscores) {
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

void show_hiscorelist5(int x, int y, int i)
{
	char s[20];
	int end, n;
	if (!hiscores_read)
		readhiscores(NULL);
	sethiscorecontext();
	if (!num_hiscores)
		return;
	setcurs(x, y);
	i -= 5;
	if (i < 0)
		i = 0;
	end = i+5;
	if (end > num_hiscores)
		end = num_hiscores;
	while (i < end) {
		n = sprintf(s, "%2d. %s", i+1, gethiscorename(i, s+12));
		memset(s+n, ' ', 12-n);
		gethiscorestr(i, s+12);
		printstr(s);
		newln(x);
		i++;
	}
}
