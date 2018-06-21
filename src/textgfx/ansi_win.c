#include "textgfx.h"
#include "ansivt.h"
#include "../game/tetris.h"
#include "../draw/draw.h"

static int window;

void setwcurs(int win, int x, int y)
{
	if (win != window) {
		getwin_xy(win, &win_x, &win_y);
		window = win;
	}
	setcurs(x, y);
}

void refreshwin(int win)
{
	static int redraw;
	struct player *p;
	if (redraw)
		return;
#ifdef TWOPLAYER
	if ((win == 1 || win == 2) && !game_running) {
		p = &game->player[win-1];
#else
	if (win == 1 && !game_running) {
		p = &player1;
#endif
		redraw = 1;
		if (game_paused)
			redrawboard(p, 19);
		else
			redrawboard(p, 3);
		redraw = 0;
	}
	refreshscreen();
}

/* only used for clearing window showing next tetromino and game screen */
void clearwin(int win)
{
	int h;
	setwcurs(win, 0, 0);
	if (win >= WIN_NEXT)
		clearbox(0, 0, 8, 2);
	else {
		margin_x = 0;
		h = term_height;
		if (h < 24 && h > 21)
			h = 21;
		clearbox(0, 0, 0, h);
	}
}

void setcolorpair(int clr)
{
	int bg;
	if (clr != PANEL_LABEL_COLOR)
		set_color_pair(clr);
	else if (!_MONOCHROME) {
		clr = 3;
		if (TWOPLAYER_MODE)
			bg = 4;
		else {
			bg = (player1.level % 6)+1;
			if (bg==6)
				clr = 7;
		}
		set_ansi_color(bg, clr, '1');
	}
}

#ifndef NO_MENU
void textgfx_entermenu()
{
	while (curs_y < 4)
		newln(0);
	menuheight = 4;
	margin_x = getmargin_x();
	draw_tetris_logo(0, 0);
}
#endif

void textgfx_entergame()
{
	if (menuheight) {
		menuheight = 0;
		clearwin(0);
		margin_x = getmargin_x();
	}
}
