#include "../config.h"
#include <allegro.h>
#include "textgfx.h"
#include "alleg.h"
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
	int x, y, y2, w, h;
	if (redraw)
		return;
	if (!game || !game_running) {
		if (win == 1 || win == 2) {
			p = &game->player[win-1];
			redraw = 1;
			if (game_paused)
				redrawboard(p, 19);
			else
				redrawboard(p, 3);
			redraw = 0;
		}
		refreshscreen();
		return;
	}
	if (!refresh_needed)
		return;
	x = 8 *blit_rect[0];
	y = 16*blit_rect[1];
	w = 8 *(blit_rect[2]-blit_rect[0]+1);
	h = 16*(blit_rect[3]-blit_rect[1]+1);
	y2 = y;
	if (!is_windowed_mode())
		y2 += 40;
	acquire_screen();
	blit(virt_screen, screen, x, y, x, y2, w, h);
	release_screen();
	refresh_needed = 0;
	memset(blit_rect, 0, 4);
}

void clearwin(int win)
{
	int h;
	setwcurs(win, 0, 0);
	if (win >= WIN_NEXT)
		clearbox(0, 0, 8, 2);
	else {
		clear_bitmap(virt_screen);
		refresh_needed = 1;
	}
}

void setcolorpair(int clr)
{
	int bg;
	if (clr == PANEL_LABEL_COLOR && !_MONOCHROME) {
		clr = 3;
		if (game->mode & MODE_2PLAYER)
			bg = 4;
		else {
			bg = (player1.level % 6)+1;
			if (bg==6)
				clr = 7;
		}
		clr = bg | clr<<3 | 64;
	}
	set_color_pair(clr);
}

void textgfx_entermenu()
{
	clear_bitmap(virt_screen);
	margin_x = getmargin_x();
	win_y = 1;
	draw_tetris_logo(0, 0);
	print_vitetris_ver(19, 4);
}

void textgfx_entergame()
{
	clearwin(0);
	margin_x = getmargin_x();
	if (!window)
		win_y = 0;
	rectfill(screen, 8, SCREEN_H-16, 300, SCREEN_H-1, 0);
}
