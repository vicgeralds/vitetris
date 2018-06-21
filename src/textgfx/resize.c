#include <signal.h>
#include "textgfx.h"
#if !SIGWINCH
void enable_term_resizing() {}
void upd_termresize() {}
#else

#ifdef CURSES
#include <curses.h>
#include "curs.h"
#else
#include "ansivt.h"	/* menuheight */
#endif
#include "../game/tetris.h"
#include "../draw/draw.h"
#include "../netw/sock.h"

static void resizehandler(int sig)
{
	textgfx_flags |= TERM_RESIZED;
}

void enable_term_resizing()
{
	signal(SIGWINCH, resizehandler);
}

static void entergame_redraw()
{
#ifndef CURSES
	menuheight = 1;
#endif
	textgfx_entergame();
	if (!TWOPLAYER_MODE)
		drawgamescreen_1p();
	else
		drawgamescreen_2p();
	if (game->next) {
		refreshwin(0);
		print_press_key();
	}
#ifdef SOCKET
	if (game->mode & MODE_NETWORK && CONNECTED !=
				(sock_flags & (CONNECTED | WAIT_PL2INGAME)))
		print_game_message(1, "WAIT", 1);
#endif
}

#if CURSES && KEY_RESIZE
static void resizemenu(int w, int h)
{
	int x, y;
	if (h > term_height) {
		getyx(window, y, x);
		wmove(window, term_height-2, 0);
		wclrtobot(window);
		wmove(window, y, x);
	}
	x = getmargin_x();
	if (w > term_width)
		mvwin(window, 1, x);
	resizeterm(term_height, term_width);
	wresize(window, term_height-1, term_width-x);
	if (w < term_width)
		mvwin(window, 1, x);
	clear();
	refresh();
	touchwin(window);
	wrefresh(window);
}

static void resizegamerunning()
{
	WINDOW *next = wins[WIN_NEXT];
	WINDOW *next2 = NULL;
	int i, x, y;
	wins[WIN_NEXT] = NULL;
	if (TWOPLAYER_MODE) {
		next2 = wins[WIN_NEXT+1];
		wins[WIN_NEXT+1] = NULL;
	}
	entergame_redraw();
	touchwin(wins[1]);
	wrefresh(wins[1]);
	if (TWOPLAYER_MODE) {
		touchwin(wins[2]);
		wrefresh(wins[2]);
	}
	if (!next)
		return;
	i = WIN_NEXT;
	while (1) {
		getwin_xy(i, &x, &y);
		x += getmargin_x();
		mvwin(next, y, x);
		overlay(next, wins[i]);
		delwin(next);
		wrefresh(wins[i]);
		if (TWOPLAYER_MODE && next != next2) {
			next = next2;
			i++;
			continue;
		}
		break;
	}
}
#endif

void upd_termresize()
{
#if !CURSES || KEY_RESIZE
	int w, h;
	if (!(textgfx_flags & TERM_RESIZED) ||
	    !in_menu && game && game->state & (GAME_OVER
# ifndef CURSES
					| GAME_RUNNING
# endif
					))
		return;
	textgfx_flags &= ~TERM_RESIZED;
	w = term_width;
	h = term_height;
	gettermsize();
# ifndef CURSES
	if (w == term_width && h == term_height)
		return;
	if (in_menu)
		return;
# else
	if (in_menu) {
		resizemenu(w, h);
		return;
	}
	resizeterm(term_height, term_width);
	if (game && game_running) {
		resizegamerunning();
		return;
	}
# endif
	entergame_redraw();
# ifndef CURSES
	if (term_height < 24 && term_height > 21) {
		w = -getmargin_x();
		setwcurs(0, -w, 20);
		newln(-w);
		clearbox(w, 21, 0, term_height-21);
	}
# endif
	drawnext(&player1, game->next);
	if (TWOPLAYER_MODE)
		drawnext(&player2, game->next);
#endif
}
#endif /* SIGWINCH */
