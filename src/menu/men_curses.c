#include <string.h>
#include <curses.h>
#include "menu.h"
#include "menuext.h"
#include "../textgfx/textgfx.h"
#include "../textgfx/curs.h"
#include "../draw/draw.h"
#include "../game/tetris.h"

#ifdef INET
#define STARTUP_N 7
#else
#define STARTUP_N 6
#endif
#define STARTUP_Y (15-STARTUP_N)

static void printmenuhelp(int y)
{
	if (term_height < y+5 || term_width < 48)
		return;
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
			setcurs(5, STARTUP_Y-2);
			wclrtobot(window);
			if ((i = menu_checkinvit(1, 7)) == -1) {
				refresh();
				setwcurs(0, 5, 7);
				i = 1;
				continue;
			}
			if (!i)
				break;
		}
		move(term_height-1, 0);
		clrtoeol();
		refresh();
# endif
		setcurs(5, STARTUP_Y-2);
		wclrtobot(window);
		break;
	}
	return i;
}
#endif /* TWOPLAYER */

int gamemenu()
{
	int i = 0;
#ifdef TWOPLAYER
	int y = 7;
	while (1) {
		if (!TWOPLAYER_MODE) {
			if (term_height >= 23)
				show_hiscorelist5(5, GAMEMENU_LENGTH+9, 0);
		} else if (term_height < 23) {
			setcurs(1, 4);
			cleartoeol();
			y = 5;
		}
		i = game_menu(0, 1, y);
		if (i != 4)
			break;
		wclrtobot(window);
		inputsetup_box(0, 1, 7);
	}
	if (!i) {
		setcurs(1, 4);
		wclrtobot(window);
		print_vitetris_ver(19, 4);
	}
#else
	while (1) {
		printmenuhelp(19);
		i = game_menu(i, 1, 7);
		if (!i)
			break;
		clearbox(0, 7, 0, GAMEMENU_LENGTH);
		if (i != 4 && i < GAMEMENU_LENGTH-1)
			break;
		wclrtobot(window);
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
	int y = 7;
	if (term_height < 22)
		y--;
	show_hiscorelist(1, y);
	clearbox(0, y, 0, 12);
}

static void help_cmdline()
{
	if (term_height > 22) {
		setcurs(1, 20);
		printstr("-help for command-line options");
	}
}

static void draw_options_box()
{
	drawbox(1, 7, 31, 10+!(textgfx_flags & CYGWIN), "Options");
	help_cmdline();
}

static int op_handler(int k, int *pos)
{
	const struct termopt opts[2] = {
		{ WHITE_BG, "black white", "bg" },
		{ ASCII, "VT100 ASCII", "drawing" }
	};
	int i = *pos-2;
	if (i < 0) {
		if (k) {
			setcurs(0, 16);
			wclrtobot(window);
			inputsetup_box(k-1, 1, 7);
		}
		draw_options_box();
		return 1;
	}
	if (!i && !k) {
		getyx(window, i, k);
		setcurs(2, i+5+!(textgfx_flags & CYGWIN));
		putnchars(HLINE, 28);
		help_cmdline();
		setcurs(k, i);
		i = k = 0;
	}
	if (i < 1+!(textgfx_flags & CYGWIN))
		i = term_optionhandler(k, opts+i);
#ifndef NO_BLOCKSTYLES
	else
		i = select_blockstyle(k);
#endif
	if (i == 3) {
		draw_tetris_logo(0, 0);
		if (!_MONOCHROME) {
			init_color_pairs();
			if (*pos == 2)
				clearok(window, TRUE);
		}
		draw_options_box();
	}
	return i;
}

void optionsmenu()
{
	const char *items[2] = {"Term BG", "Line Drawing"};
	int n = 1+!(textgfx_flags & CYGWIN);
	draw_options_box();
	options_menu(items, n, op_handler, 3, 9);
	setcurs(0, 7);
	wclrtobot(window);
}
