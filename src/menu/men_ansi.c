#include <stdio.h>
#include "menu.h"
#include "menuext.h"
#include "../textgfx/textgfx.h"
#include "../textgfx/ansivt.h"
#include "../draw/draw.h"
#include "../game/tetris.h"
#include "../lang.h"

#ifdef TWOPLAYER
int startupmenu(int i)
{
	while (1) {
		print_vitetris_ver(19, 4);
		newln(0);
		newln(19);
		printstr("Written by"); newln(19);
		printstr("Victor Nilsson"); newln(19);
		printstr("2007-2009");
		i = startup_menu(i-1, 1, 4);
# ifndef TTY_SOCKET
		if (!i)
			break;
# else
		if (i==2 && !select_2p_tty(17, 5))
			continue;
		if (!i && (i = menu_checkinvit(1, 4)) == -1) {
			i = 1;
			continue;
		}
		if (!i)
			break;
		setcurs_end();
		printf("\033[K");
# endif
		clearbox(0, 4, 0, menuheight-4);
		break;
	}
	return i;
}
#endif /* TWOPLAYER */

int gamemenu()
{
	int i = 1;
#ifdef TWOPLAYER
	int h;
	if (!TWOPLAYER_MODE || game->mode & MODE_NETWORK)
		h = GAMEMENU_LENGTH+4;
	else {
		h = 17;
		while (menuheight < h)
			newln(2);
	}
	while (1) {
		i = game_menu(0, 1, 4);
		if (i==4)
			inputsetup_box(0, 1, 4);
		else
			break;
	}
	if (!i)
		clearbox(0, 4, 0, h-4);
#else
	while (1) {
		i = game_menu(i-1, 1, 4);
		if (i==4) {
			setcurs(0, 13);
			printf("\033[K");
			inputsetup_box(0, 1, 4);
			i = 1;
		} else if (i < GAMEMENU_LENGTH-1)
			break;
		else {
			clearbox(0, 4, 0, GAMEMENU_LENGTH);
			if (i == GAMEMENU_LENGTH-1)
				optionsmenu();
			else
				hiscorelist();
		}
	}
#endif
	return i;
}

#ifdef INET
int netplaymenu()
{
	return netplay_menu(1, 4);
}
#endif

void hiscorelist()
{
	menuheight = 6;
	show_hiscorelist(1, 4);
	clearbox(0, 4, 0, menuheight-4);
}

static int op_handler(int k, int *pos)
{
	const struct termopt opts[3] = {
		{ WHITE_BG, "black white", "bg" },
		{ MONOCHROME, "16 mono", "color" },
#ifdef IBMGRAPHICS
		{ ASCII, "CP437 ASCII", "drawing" }
#else
		{ ASCII, "VT100 ASCII", "drawing" }
#endif
	};
	int i = *pos-2;
	if (i < 0) {
		if (k)
			inputsetup_box(k-1, 1, 4);
		return 1;
	}
	if (!i && !k)
		menuheight = 11;
	if (i < 3)
		i = term_optionhandler(k, opts+i);
#ifndef NO_BLOCKSTYLES
	else
		i = select_blockstyle(k);
#endif
	if (i == 3)
		draw_tetris_logo(0, 0);
	return i;
}

void optionsmenu()
{
	const char *items[3] = {"Term BG", NULL, "Line Drawing"};
	char colors[8] = "Colours";
	spellword(colors);
	items[1] = colors;
	options_menu(items, 3, op_handler, 1, 4);
	menuheight = 8;
}
