#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../config.h"
#if HAVE_GETTEXTINFO
#include <conio.h>
#define conio_h
#elif UNIX
#include <unistd.h>
#include <sys/ioctl.h>
#endif
#include "textgfx.h"
#include "../options.h"

char term_width = 80;
char term_height = 20;

void blockstyle_from_option(const struct option *o);

static int gettermsize_unix()
{
#if defined(TIOCGSIZE)
	struct ttysize tty;
	if (!ioctl(STDIN_FILENO, TIOCGSIZE, &tty)) {
		settermwidth (tty.ts_cols);
		settermheight(tty.ts_lines);
		return 1;
	}
#elif defined(TIOCGWINSZ)
	struct winsize win;
	if (!ioctl(STDIN_FILENO, TIOCGWINSZ, &win)) {
		settermwidth (win.ws_col);
		settermheight(win.ws_row);
		return 1;
	}
#endif
	return 0;
}

void gettermsize()
{
	char *s;
	int w = 80;
	int h = 25;
#if HAVE_GETTEXTINFO
	struct text_info text;
	gettextinfo(&text);
	w = text.screenwidth;
	h = text.screenheight;
#endif
	if (gettermsize_unix())
		return;
	if (s = getenv("COLUMNS"))
		w = atoi(s);
	if (s = getenv("LINES"))
		h = atoi(s);
	settermwidth(w);
	settermheight(h);
}

void settermwidth(int w)
{
	if (w < 32)
		w = 32;
	else if (w > 127)
		w = 127;
	term_width = w;
}

void settermheight(int h)
{
	if (h < 20)
		h = 20;
	else if (h > 25)
		h = 25;
	term_height = h;
	if (term_height < 24)
		textgfx_flags &= ~HEIGHT_24L;
	else
		textgfx_flags |= HEIGHT_24L;
}

static void gettermenv()
{
#if UNIX
	char *s = getenv("TERM");
	if (s) {
		if (!strncmp(s, "xterm", 5))
			textgfx_flags |= XTERM;
		else if (!strcmp(s, "linux"))
			textgfx_flags |= LINUX_TERM;
		else if (!strcmp(s, "cygwin"))
			textgfx_flags |= CYGWIN;
	}
	s = getenv("COLORTERM");
	if (s && !strcmp(s, "gnome-terminal"))
		textgfx_flags |= GNOME_TERM;
#endif
}

void gettermoptions()
{
	struct option *o;
	char *k;
	union val v;
	gettermenv();
	o = getoptions("term");
	for (; o; o = o->next) {
		k = opt_key(o);
		v = o->val;
		if (!strcmp(k, "bg")) {
			if (v.integ == 1)
				textgfx_flags |= WHITE_BG;
		} else if (!strcmp(k, "drawing")) {
			if (v.integ)
				textgfx_flags |= ASCII;
			else
				textgfx_flags &= ~ASCII;
		} else if (!strcmp(k, "color")) {
			if (!v.integ)
				textgfx_flags |= MONOCHROME;
		} else
			blockstyle_from_option(o);
	}
	if ((textgfx_flags & (WHITE_BG | TT_MONO))==(WHITE_BG | TT_BLOCKS))
		textgfx_flags ^= TT_BLOCKS | TT_BLOCKS_BG;
	reset_block_chars();
}
