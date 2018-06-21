#include <curses.h>
#include "textgfx.h"
#include "curs.h"
#include "../game/tetris.h"
#include "../draw/draw.h"

static int winindex(int win)
{
	switch (win) {
	case WIN_TETROM_STATS:
		return 2;
	case WIN_TOP_SCORES:
		return 4;
	}
	return win;
}

static void clearboard_paused_fix()
{
#ifdef PDCURSES
	int x = board_x(1, 0);
	int i;
	setcurs(x, _HEIGHT_24L ? 4 : 0);
	for (i=0; i < 20; i++) {
		putnchars('/', 20);
		if (i < 19)
			newln(x);
	}
	refresh();
#endif
}

void setwcurs(int win, int x, int y)
{
	window = wins[winindex(win)];
	if (!window) {
		window = stdscr;
		margin_x = getmargin_x();
		if (game && game_paused && y <= 4)
			clearboard_paused_fix();
	}
	setcurs(x, y);
}

void refreshwin(int i)
{
	WINDOW *win;
	if (i == -1)
		win = window;
	else
		win = wins[winindex(i)];
	if ((i==1 || i==2) && !game_running) {
#ifdef PDCURSES
		print_game_message(i, "                ", 0);
#endif
		touchwin(win);
	}
	if (win)
		wrefresh(win);
	else {
		attrset(A_NORMAL);
		refresh();
	}
}

void clearwin(int win)
{
	setwcurs(win, 0, 0);
	werase(window);
}

static int set_panel_label_color()
{
	short f = COLOR_YELLOW;
	short b;
	if (TWOPLAYER_MODE)
		b = COLOR_BLUE;
	else {
		b = COLOR_1_6(player1.level % 6);
		if (b == COLOR_CYAN)
			f = COLOR_WHITE;
		if (b == COLOR_YELLOW)
			return 3;
	}
	initpair(PANEL_LABEL_COLOR, f, b);
	return PANEL_LABEL_COLOR;
}

void setcolorpair(int pair)
{
	if (pair == PANEL_LABEL_COLOR)
		pair = set_panel_label_color();
	set_color_pair(pair);
}

#ifndef NO_MENU
void textgfx_entermenu()
{
	int x = getmargin_x();
	delwins();
	refresh();
	window = newwin(term_height-1, term_width-x, 1, x);
	wins[0] = window;
	draw_tetris_logo(0, 0);
	print_vitetris_ver(19, 4);
}

static void print_ver_author()
{
	int y;
	if (_HEIGHT_24L && margin_x > 14) {
		window = stdscr;
		attrset(A_NORMAL);
		print_vitetris_ver(-margin_x, 0);
		y = term_height-3;
		mvaddstr(y,  0, "Written by");
		mvaddstr(y+1,0, "Victor Nilsson");
		mvaddstr(y+2,0, "2007-2009");
	}
}
#endif

static void createwin(int i, int w, int h)
{
	int x, y;
	getwin_xy(i, &x, &y);
	i = winindex(i);
	wins[i] = newwin(h, w, y, x+margin_x);
}

void textgfx_entergame()
{
	delwins();
	margin_x = getmargin_x();
	createwin(1, 20, 20);
	if (!TWOPLAYER_MODE) {
		createwin(WIN_NEXT, 8, 2);
		createwin(WIN_PANEL, 10, 20);
		if (term_width >= 45) {
			createwin(WIN_TETROM_STATS, 9, 9);
			if (term_width >= 47)
				createwin(WIN_TOP_SCORES, 11, 7);
		}
	} else {
		createwin(2, 20, 20);
		if (_HEIGHT_24L || term_width >= 76) {
			createwin(WIN_NEXT, 8, 2);
			createwin(WIN_NEXT+1, 8, 2);
		}
		createwin(WIN_PANEL, 12, 20);
	}
	clear();
#ifndef NO_MENU
	print_ver_author();
#endif
}
