#include <string.h>
#include "menu.h"
#include "internal.h"
#include "../input/input.h"
#include "../textgfx/textgfx.h"
#include "../draw/draw.h"
#include "../lang.h"

#if TWOPLAYER
int player_ = 0;
#endif

static int inp_keypr(int i)
{
	const char keypr[11] = {MVUP, MVDOWN, MVLEFT, MVRIGHT, A_BTN, B_BTN,
				MVUP, A_BTN, B_BTN, HARDDROP, MVDOWN};
	int k = keypr[i];
	if (player_)
		k |= player_==1 ? PLAYER_1 : PLAYER_2;
	if (i >= 6)
		k |= IN_GAME;
	return k;
}

void inp_printkeys(int dev, int x, int y)
{
	const char *k;
	int i;
	x += 6;
	setcurs(x, y);
	for (i = 0; i < 11; i++) {
		k = getkeyfor_str(dev, inp_keypr(i));
		printstr(k);
		putnchars(' ', 6-strlen(k));
		if (i == 5) {
			x += 17;
			setcurs(x, y);
		} else
			newln(x);
	}
}

static int inp_setcurs(int x, int y, int i)
{
	if (i >= 6) {
		x += 12;
		i -= 6;
	}
	setcurs(x, y+i);
	return x;
}

void inputsetup_menu(int pl, int x, int y)
{
	const char *menu[11] = {
		" Up", "Dwn", "Lft", "Rgt", "  A", "  B",
		" Rotate ", " Rot cw ", NULL, "Harddrop", "Softdrop"
	};
	char rot_acw[9] = " Rot acw";
	int dev = 0;
	int i = 0;
	int xx, k;
#ifdef JOYSTICK
	int devlist = num_joyst;
	if (devlist) {
		i = -1;
		setcurs(x, y);
		newln(x);
		y++;
	}
#endif
	spellword(rot_acw+5);
	menu[8] = rot_acw;
	drawmenu(menu, 6, i, x, y, NULL);
	drawmenu(menu+6, 5, -1, x+12, y, NULL);
#ifdef TWOPLAYER
	player_ = pl;
#endif
	while (1) {
#ifdef JOYSTICK
		if (devlist && i==-1) {
			if (!inp_devlist(&dev, x, y-1))
				return;
			printmenuitem(menu[0], 1);
			i = 0;
		}
#endif
		inp_printkeys(dev, x, y);
		xx = inp_setcurs(x, y, i);
		refreshwin(-1);
		k = setkeymapping(dev, inp_keypr(i));
#ifdef JOYSTICK
		if (devlist && (k==MVUP && !i || k=='\t' && i==10)) {
			printmenuitem(menu[i], 0);
			i = -1;
			continue;
		}
#endif
		if (k==MVUP && !i)
			k = MVDOWN;
		else if (k==MVDOWN && i==10) {
			printmenuitem(menu[10], 0);
			inp_setcurs(x, y, i=0);
			printmenuitem(menu[0], 1);
			continue;
		}
		switch (handle_menuitem_2cols(menu, 11, &i, 6,
					      xx, y+i%6, NULL, k)) {
		case 0:
		case 2:
			return;
		case 3:
			inp_setcurs(x, y, i);
			printmenuitem(menu[i], 1);
		}
	}
}

#ifndef NO_MENU
static void printhelp(int x, int y)
{
	setcurs(x, y);
	printstr("ESC to clear, TAB to skip,");  newln(x);
	printstr("ENTER to return");
}

void inputsetup_box(int pl, int x, int y)
{
	int h = 8;
#ifdef JOYSTICK
	if (num_joyst)
		h++;
#endif
	clearbox(x-1, y, 0, h+4);
	printhelp(x+2, y+h+2);
	setcurs(x, y);
	printmenuitem("Input Setup", 1);
#ifdef TWOPLAYER
	if (!pl)
		printstr(" single");
	printstr(" player");
	if (pl)
		putch(pl+'0');
#endif
	drawbox(x, y+1, 31, h, NULL);
	inputsetup_menu(pl, x+1, y+2);
	clearbox(x, y, 0, h+4);
}
#endif
