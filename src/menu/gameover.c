#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "menu.h"
#include "../input/input.h"
#include "../textgfx/textgfx.h"
#include "../hiscore.h"
#include "../draw/draw.h"
#include "../game/tetris.h"

void show_hiscorelist5(int x, int y, int i)
{
	char s[20];
	const struct hiscore *hs, *end;
	int n;
	if (!hiscores[0].score && !readhiscores(NULL))
		return;
	setcurs(x, y);
	hs = hiscores;
	i -= 5;
	if (i < 0)
		i = 0;
	else
		hs += i;
	end = hs+5;
	while (hs->score && hs != end) {
		n = sprintf(s, "%2d. %s", i+1, gethiscorename(i, s+12));
		memset(s+n, ' ', 12-n);
		sprintf(s+12, "%7ld", (long) hs->score);
		printstr(s);
		newln(x);
		hs++;
		i++;
	}
}

static void printsavebutton(int sel)
{
	if (_MONOCHROME) {
		if (sel)
			setattr_standout();
		else
			setattr_bold();
	} else if (sel) {
		setcolorpair(YELLOW_ON_GREEN);
	} else {
		setcolorpair(4);
		putch('[');
		setcolorpair(YELLOW_ON_BLUE);
		printstr(" Save ");
		setcolorpair(4);
		putch(']');
		goto out;
	}
	printstr("[ Save ]");
out:	setattr_normal();
}

static int hiscore_editname(char *name, int pos, int k)
{
	const char chars[18] = "Z.!?-0123456789 A";
	const char lat1_chars[5] = "ÅÄÖÜ";
	const char lat1_chars_small[5] = "åäöü";
	int c, i;
	char *p;
	if (k == MVLEFT && pos)
		pos--;
	else if (k == '\b' && pos) {
		pos--;
		memmove(name+pos, name+pos+1, 6-pos);
		name[6] = ' ';
	} else if (pos == 7)
		return pos;
	else if (k == MVRIGHT)
		pos++;
	else if (k == '\t')
		pos = 7;
	else if (k == 0x7F) {
		memmove(name+pos, name+pos+1, 6-pos);
		name[6] = ' ';
	} else {
		c = 0;
		i = 0;
		if (k >= 'A' && k <= 'Z' || strchr(chars, k) ||
					    strchr(lat1_chars, k))
			c = k;
		else if (k >= 'a' && k <= 'z')
			c = k-32;
		else if (p = strchr(lat1_chars_small, k))
			c = lat1_chars[p-lat1_chars_small];
		else if (k == MVUP || k == A_BTN || k == B_BTN) {
			c = name[pos];
			i = (k==B_BTN) ? -1 : 1;
			if (c > 'A'-(i>0) && c < 'Z'+(i<0))
				c += i;
			else
				c = *(strchr(chars, c)+i);
		}
		if (c) {
			name[pos] = c;
			if (!i)
				pos++;
		}
	}
	return pos;
}

static int hiscore_entername_menu(char *name, const char **menu, int x, int y)
{
	int pos = 0;
	int i = 0;
	int k;
	while (1) {
		drawmenu(menu, 2, i-2, x, y+2, NULL);
textbox:	setcurs(x, y);
		printtextbox(name, pos);
		movefwd(3);
		printsavebutton(!i && pos==7);
		if (!i)
			x += pos;
		setcurs(x, y+i);
		refreshwin(-1);
		autorep_a_b_btns = 1;
		k = getkeypress_block(SINGLE_PL) & 0xFF;
		autorep_a_b_btns = 0;
		if (k == ESC)
			return 0;
		if (!i) {
			x -= pos;
			switch (k) {
			case STARTBTN:
				return 1;
			case '\t':
				if (pos < 7)
					break;
			case MVDOWN:
				i = 2;
				continue;
			}
			pos = hiscore_editname(name, pos, k);
			goto textbox;
		}
		switch (k) {
		case STARTBTN:
		case A_BTN:
			if (i==2)
				return 2;
		case 'q':
			exit(0);
		case MVUP:
			i = i==2 ? 0 : 2;
			break;
		case '\t':
			i = i==2 ? 3 : 0;
			break;
		case MVDOWN:
			if (i < 3)
				i++;
		}
		if (!i && pos==7) {
			pos = 6;
			while (pos && name[pos-1]==' ')
				pos--;
		}
	}
}

static int playagain_menu(const char **menu, int x, int y)
{
	int ret = openmenu(menu, 2, 0, x, y, NULL);
	if (ret==2)
		exit(0);
	return ret;
}

static int hiscorebox(const char **menu, int x, int y)
{
	int i = 0;
	while (i < 10 && hiscores[i].score >= player1.score)
		i++;
	if (i < 10)
		i++;
	while (is_outside_screen(x+24, 0))
		x--;
	drawbox(x, y, 24, 11, "HIGHSCORES");
	x += 2;
	show_hiscorelist5(x, y+2, i);
	if (playagain_menu(menu, x+2, y+8)) {
		clearbox(x+21, y, 0, 11);
		return 1;
	}
	return 0;
}

static int hiscore_congrats(const char **menu)
{
	char name[8] = "       ";
	int x = 9;
	int y = _HEIGHT_24L ? 7 : 3;
	setwcurs(0, 9, y);
	while (is_outside_screen(x+26, 0))
		x--;
	drawbox(x, y, 26, 9, "CONGRATULATIONS!");
	setcurs(x+2, y+2);
	printstr("You have a highscore!");  newln(x+2);
	printstr("Please enter your name");
entername:
	switch (hiscore_entername_menu(name, menu, x+4, y+4)) {
	case 0:
		return 0;
	case 1:
		if (savehiscore(name)) {
			if (x > 7)
				clearbox(33, y, 2, 9);
			return hiscorebox(menu, 9, y);
		}
		setcurs(x+2, y+2);
		printstr("ERROR! Could not save");  newln(x+2);
		printstr("score to file.        ");
		goto entername;
	case 2:
		clearbox(32, y, 0, 9);
		return 1;
	}
}

int gameovermenu()
{
	const char *menu[2] = {"Play again", "Exit"};
	readhiscores(NULL);
	if (ishiscore())
		return hiscore_congrats(menu);
	setwcurs(1, 2, 3);
	drawbox(2, 3, 17, 5, "GAME OVER");
	return playagain_menu(menu, 4, 5);
}
