#include <string.h>
#include "../config.h"
#include <allegro.h>
#include "menu.h"
#include "menuext.h"
#include "../textgfx/textgfx.h"
#include "../draw/draw.h"
#include "../game/tetris.h"
#include "../input/input.h"
#include "../options.h"

#ifdef INET
#define STARTUP_N 7
#else
#define STARTUP_N 6
#endif
#define STARTUP_Y (15-STARTUP_N)

static void printmenuhelp(int y)
{
	setcurs(0, y);
	printstr("Use the arrow keys or TAB to navigate,"); newln(0);
	printstr("ENTER to proceed, BACKSPACE or ESC to");  newln(0);
	printstr("go back.  Exit at once with Q.");
}

#ifdef TWOPLAYER
int startupmenu(int i)
{
	while (1) {
		drawbox(5, STARTUP_Y-2, 22, STARTUP_N+4, NULL);
		printmenuhelp(19);
		i = startup_menu(i-1, 8, STARTUP_Y);
# ifndef TTY_SOCKET
		if (!i)
			break;
# else
		if (i==2 && !select_2p_tty(24, STARTUP_Y+1))
			continue;
		if (!i) {
			clearbox(0, 6, 0, 16);
			if ((i = menu_checkinvit(1, 7)) == -1) {
				i = 1;
				continue;
			}
			if (!i)
				break;
		}
		setcurs_end();
		cleartoeol();
		textgfx_entermenu();
# endif
		clearbox(0, 6, 0, 16);
		break;
	}
	return i;
}
#endif /* TWOPLAYER */

int gamemenu()
{
	int i = 0;
#ifdef TWOPLAYER
	while (1) {
		if (!TWOPLAYER_MODE)
			show_hiscorelist5(5, GAMEMENU_LENGTH+9, 0);
		i = game_menu(0, 1, 7);
		if (i != 4)
			break;
		clearbox(5, GAMEMENU_LENGTH+9, 0, 5);
		inputsetup_box(0, 1, 7);
	}
	if (!i)
		clearbox(0, 7, 0, 14);
#else
	while (1) {
		printmenuhelp(19);
		i = game_menu(i, 1, 7);
		if (!i)
			break;
		clearbox(0, 7, 0, GAMEMENU_LENGTH);
		if (i != 4 && i < GAMEMENU_LENGTH-1)
			break;
		clearbox(0, 19, 0, 3);
		switch (i) {
		case 4:
			inputsetup_box(0, 1, 7);
			i = 0;
			continue;
		case GAMEMENU_LENGTH-1:
			optionsmenu();
			break;
		default:
			hiscorelist();
		}
		i--;
	}
#endif
	return i;
}

#ifdef INET
int netplaymenu()
{
	return netplay_menu(1, 7);
}
#endif

void hiscorelist()
{
	show_hiscorelist(1, 7);
	clearbox(0, 7, 0, 12);
}

static void help_alt_enter()
{
	setcurs(2, 19);
	printstr("ALT-ENTER toggles fullscreen");
	cleartoeol();
}

static void draw_options_box()
{
	drawbox(1, 7, 31, 11, "Options");
	help_alt_enter();
}

static int op_handler(int k, int *pos)
{
	const struct termopt opt = {ASCII, "CP437 ASCII", "drawing"};
	int i = *pos-2;
	if (i < 0) {
		if (k) {
			clearbox(0, 16, 0, 4);
			inputsetup_box(k-1, 1, 7);
		}
		draw_options_box();
		return 1;
	}
	if (!i) {
		if (k == MVLEFT)
			i = 0;
		else if (k == MVRIGHT)
			i = 1;
		else {
			i = !getopt_int("", "fullscreen");
			if (k == A_BTN)
				i = !i;
			else if (k)
				return 0;
		}
		printmenuitem_options("yes no", i);
		if (k) {
			union val v;
			v.integ = !i;
			setoption("", "fullscreen", v, 0);
		}
		i = 1;
	} else if (i == 1) {
		if (!k) {
			get_xy(&k, &i);
			setcurs(2, i+5);
			putnchars(HLINE, 28);
			help_alt_enter();
			setcurs(k, i);
			k = 0;
		}
		i = term_optionhandler(k, &opt);
	}
#ifndef NO_BLOCKSTYLES
	else
		i = select_blockstyle(k);
#endif
	if (i == 3) {
		draw_tetris_logo(0, 0);
		draw_options_box();
	}
	return i;
}

void optionsmenu()
{
	const char *items[2] = {"Fullscreen", "Line Drawing"};
	draw_options_box();
	options_menu(items, 2, op_handler, 3, 9);
	clearbox(1, 7, 0, 11);
}
