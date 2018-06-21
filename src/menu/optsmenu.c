#include <string.h>
#include "menu.h"
#include "menuext.h"
#include "../input/input.h"
#include "../textgfx/textgfx.h"
#include "../draw/draw.h"
#include "../options.h"
#include "../lang.h"

#ifdef TWOPLAYER
static int op_inputsetup_select(int x, int y)
{
	const char *items[3] = {
		"single player",
		"  player1",
		"  player2"
	};
	return dropdownlist(items, 3, 0, x+14, y);
}
#else
#define op_inputsetup_select(x, y) 1
#endif

static void save_bgdot()
{
	union val v;
	v.str[0] = bgdot;
	v.str[1] = '\0';
	setoption("term", "bgdot", v, 1);
}

static int op_bgdot(int k, int *pos)
{
	const char *items[3] = {"dot", "bullet", "blank"};
	int n = 3;
	int i = 0;
	int j;
	if (isprintable(k)) {
		if (k == '0')
			bgdot = ' ';
		else if (k == '~')
			bgdot = BULLET;
		else
			bgdot = k;
		save_bgdot();
		k = 0;
	}
	if (_ASCII) {
		items[1] = items[2];
		n--;
	} else if (bgdot == BULLET)
		i = 1;
	if (bgdot == ' ')
		i = n-1;
	j = i;
	k = selectitem(items, n, &i, k);
	if (i != j) {
		if (i == 0)
			bgdot = '.';
		else if (i == n-1)
			bgdot = ' ';
		else
			bgdot = BULLET;
		save_bgdot();
	}
	if (k != 1)
		return k;
	putch(bgdot);
	putch(' ');
	return 1;
}

static int op_tcolor(int k, int *pos)
{
	char *c = tetrom_colors + *pos;
	if (k >= '1' && k <= '7')
		*c = k-'0';
	else if (k == A_BTN) {
		*c += 1;
		if (*c == 8)
			*c = 1;
	} else if (k)
		return 0;
	setcolorpair(*c);
	putch(' ');
	putch(*c+'0');
	putch(' ');
	setattr_normal();
	return 1;
}

static int op_tcolor_gt4(int k, int *pos)
{
	int i = *pos+4;
	return op_tcolor(k, &i);
}

static void tetrom_colors_menu(int x, int y)
{
	const char *menu[7] = {"I", "J", "L", "O", "S", "T", "Z"};
	menuhandler handlers[7] = {
		op_tcolor, op_tcolor, op_tcolor, op_tcolor,
		op_tcolor_gt4, op_tcolor_gt4, op_tcolor_gt4
	};
	int i = 0;
	if (y+6 >= term_height)
		y -= 7;
	drawbox(x, y, 19, 6, (char *) 0);
	x++;
	y++;
	drawmenu(menu, 4, 0, x, y, handlers);
	drawmenu(menu+4, 3, -1, x+9, y, handlers+4);
	autorep_a_b_btns = 1;
	while (1) {
		setcurs(x, y+i%4);
		refreshwin(-1);
		switch (handle_menuitem_2cols(menu, 7, &i, 4, x, y+i%4,
				handlers, getkeypress_block(SINGLE_PL))) {
		case 0:
		case 2:
			autorep_a_b_btns = 0;
			if (i >= 4)
				x -= 9;
			clearbox(x-1, y-1, 19, 6);
			return;
		case 3:
			x += i>=4 ? 9 : -9;
			setcurs(x, y+i%4);
			printmenuitem(menu[i], 1);
		}
	}
}

void options_menu(const char **items, int n, menuhandler f, int x, int y)
{
	const char *menu[8] = {"Input Setup", "-"};
	char tetr_colors[18] = "Tetromino Colours";
	menuhandler handlers[8] = {rarrow_menuitem};
	int i = 1;
	memcpy(&menu[2], items, n*sizeof(char *));
	while (i <= n)
		handlers[++i] = f;
#if !NO_BLOCKSTYLES
	menu[++i] = "Block Style";
	handlers[i] = f;
	n++;
#endif
	menu[++i] = "Board BG";
	handlers[i] = op_bgdot;
	spellword(tetr_colors+10);
	menu[++i] = tetr_colors;
	handlers[i] = rarrow_menuitem;
	n += 4;
	i = 0;
	while (i = openmenu(menu, n, i, x, y, handlers)) {
		i--;
		if (!i) {
			f(op_inputsetup_select(x, y), &i);
			inputdevs_player[0] = 0;
		} else if (i == n-1)
			tetrom_colors_menu(x, y+i+1);
		else if (!handlers[i](STARTBTN, &i))
			break;
	}
	clearbox(x, y, 0, n);
}

int term_optionhandler(int k, const struct termopt *o)
{
	int flag = o->flag;
	union val v;
	switch (k) {
	case 0:       break;
	case MVLEFT:  textgfx_flags &= ~flag; break;
	case MVRIGHT: textgfx_flags |=  flag; break;
	case A_BTN:   textgfx_flags ^=  flag; break;
	default:      return 0;
	}
	printmenuitem_options(o->s, (textgfx_flags & flag)!=0);
	if (!k)
		return 1;
	v.integ = flag==MONOCHROME ? 2-k : k-1;
	setoption("term", o->key, v, 0);
#if !NO_BLOCKSTYLES
	if ((textgfx_flags & (WHITE_BG | TT_MONO))==(WHITE_BG | TT_BLOCKS) ||
	    _TT_BLOCKS_BG && (!_WHITE_BG || _MONOCHROME) &&
	    getopt_int("term", "block") == -1)
		textgfx_flags ^= TT_BLOCKS | TT_BLOCKS_BG;
#endif
	reset_block_chars();
	return 3;
}
