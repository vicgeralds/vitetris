#include <string.h>
#include <curses.h>
#include "textgfx.h"
#include "curs.h"
#include "../input/termin.h"

#ifndef NCURSES_VERSION
const short colors1_6[6] = {
	COLOR_RED,  COLOR_GREEN,   COLOR_YELLOW,
	COLOR_BLUE, COLOR_MAGENTA, COLOR_CYAN
};
#endif

unsigned textgfx_flags = 0;

int margin_x = 0;

WINDOW *window;
WINDOW *wins[6] = {NULL};

static chtype acs_chars[17];

void textgfx_init()
{
	if (textgfx_flags & CYGWIN)
		textgfx_flags |= ASCII;
	initscr();
	set_input_mode();
	curs_set(0);
	if (!_MONOCHROME && start_color() != ERR && COLOR_PAIRS >= 16) {
#ifdef NCURSES_VERSION
		use_default_colors();
#endif
		init_color_pairs();
	} else
		textgfx_flags |= MONOCHROME;
	window = stdscr;
	refresh();
	acs_chars[0] = ACS_BOARD;	/* h */
	acs_chars[2] = ACS_LRCORNER;	/* j */
	acs_chars[3] = ACS_URCORNER;	/* k */
	acs_chars[4] = ACS_ULCORNER;	/* l */
	acs_chars[5] = ACS_LLCORNER;	/* m */
	acs_chars[6] = ACS_PLUS;	/* n */
	acs_chars[9] = ACS_HLINE;	/* q */
	acs_chars[12] = ACS_LTEE;	/* t */
	acs_chars[13] = ACS_RTEE;	/* u */
	acs_chars[14] = ACS_BTEE;	/* v */
	acs_chars[15] = ACS_TTEE;	/* w */
	acs_chars[16] = ACS_VLINE;	/* x */
}

void init_color_pairs()
{
	int i;
	for (i = 0; i < 6; i++)
		initpair(i+1, COLOR_1_6(i), COLOR_1_6(i));
	if (_WHITE_BG)
		initpair(7, COLOR_BLACK, COLOR_BLACK);
	else
		initpair(7, COLOR_WHITE, COLOR_WHITE);
	init_pair(MAGENTA_FG, COLOR_MAGENTA, COLOR_DEFAULT_BG);
	initpair(WHITE_ON_BLUE, COLOR_WHITE, COLOR_BLUE);
	i = _WHITE_BG ? COLOR_CYAN : COLOR_BLUE;
	init_pair(BOARD_BG_COLOR, i, COLOR_DEFAULT_BG);
	init_pair(BOARD_FRAME_COLOR, COLOR_BLUE, COLOR_DEFAULT_BG);
	init_pair(RED_FG, COLOR_RED, COLOR_DEFAULT_BG);
	if (!(textgfx_flags & (TT_BLOCKS | BLACK_BRACKETS))) {
		init_pair(15, COLOR_GREEN, COLOR_DEFAULT_BG);
		init_pair(16, COLOR_YELLOW, COLOR_DEFAULT_BG);
		init_pair(17, COLOR_CYAN, COLOR_DEFAULT_BG);
	}
}

void initpair(short pair, short f, short b)
{
	if (_TT_BLOCKS || f==b && textgfx_flags & BLACK_BRACKETS)
		b = COLOR_DEFAULT_BG;
	init_pair(pair, f, b);
}

void textgfx_end()
{
	int x, y;
	delwins();
	if (!isendwin()) {
		getmaxyx(stdscr, y, x);
		move(y-1, 0);
		clrtobot();
		refresh();
		endwin();
	}
}

void delwins()
{
	int i;
	for (i = 0; i < 6; i++)
		if (wins[i]) {
			delwin(wins[i]);
			wins[i] = NULL;
		}
}

void setcurs(int x, int y)
{
	if (window == stdscr)
		x += margin_x;
	else if (x < 0) {
		margin_x = x;
		x = 0;
	} else
		margin_x = 0;
	wmove(window, y, x);
}

void movefwd(int n)
{
	int x, y;
	getyx(window, y, x);
	x += n;
	if (margin_x < 0)
		setcurs(x+margin_x, y);
	else
		wmove(window, y, x);
}

void newln(int x)
{
	int d, y;
	getyx(window, y, d);
	setcurs(x, y+1);
}

void setcurs_end()
{
	move(term_height-1, 0);
	refresh();
	window = stdscr;
}

int is_outside_screen(int x, int y)
{
	int xmax, ymax;
	if (window == stdscr)
		x += margin_x;
	getmaxyx(window, ymax, xmax);
	return x >= xmax || y >= ymax;
}

void get_xy(int *x, int *y)
{
	getyx(window, *y, *x);
}

void cleartoeol()
{
	wclrtoeol(window);
}

static void setfgcolor(int clr)
{
	int pairs[6] = {RED_FG, 15, 16, BOARD_FRAME_COLOR, MAGENTA_FG, 17};
	int attrs = A_BOLD;
	clr &= 7;
	if (textgfx_flags & (TT_BLOCKS | BLACK_BRACKETS))
		attrs |= COLOR_PAIR(clr);
	else
		attrs |= COLOR_PAIR(pairs[clr-1]);
	wattrset(window, attrs);
}

void set_color_pair(int pair)
{
	int attrs;
	if (_MONOCHROME) {
		if (pair == MAGENTA_FG)
			setattr_bold();
		return;
	}
	if (pair == YELLOW_ON_BLUE) {
		initpair(YELLOW_ON_GREEN, COLOR_YELLOW, COLOR_BLUE);
		pair = YELLOW_ON_GREEN;
	} else if (pair == YELLOW_ON_GREEN)
		initpair(YELLOW_ON_GREEN, COLOR_YELLOW, COLOR_GREEN);
	else if (pair & 16) {
		setfgcolor(pair);
		return;
	}
	attrs = COLOR_PAIR(pair);
	switch (pair) {
	case 0:
	case BOARD_BG_COLOR:
	case BOARD_FRAME_COLOR:
		break;
	case PANEL_LABEL_COLOR:
		attrs |= A_BOLD;
		break;
	case RED_FG:
		if (_WHITE_BG)
			break;
	default:
		if (pair <= 7 && textgfx_flags & BLACK_BRACKETS)
			attrs |= A_REVERSE;
		else
			attrs |= A_BOLD;
	}
	wattrset(window, attrs);
}

void setattr_normal()
{
	wattrset(window, A_NORMAL);
}

void setattr_standout()
{
	wstandout(window);
}

void setattr_bold()
{
	if ((textgfx_flags & TT_MONO) != TT_MONO)
		wattron(window, A_BOLD);
}

void setattr_underline()
{
	wattron(window, A_UNDERLINE);
}

static int putch_acs1(int ch)
{
	chtype c;
	switch (ch) {
	case TEXTURE1:
		c = ACS_CKBOARD;
		break;
	case BULLET:
		c = ACS_BULLET;
		break;
	case UPARROW:
		c = ACS_UARROW;
		break;
	default:
		return 0;
	}
	waddch(window, c);
	return 1;
}

static int putch_acs(int ch)
{
	int x, y;
	int i;
	if (putch_acs1(ch))
		return 1;
	if (ch >= TEXTURE2 && ch <= VLINE) {
		i = ch-0x100-'h';
		if (acs_chars[i]) {
			getyx(window, y, x);
			waddch(window, acs_chars[i]);
			if (is_outside_screen(x+1, 0))
				wmove(window, y, x);
			return 1;
		}
	}
	return 0;
}

void put_ch(int ch)
{
	if (!(ch & 0x100)) 
		waddch(window, ch);
	else if (_ASCII || ch == (' '|0x100) || ch == TEXTURE2 &&
			textgfx_flags & (XTERM | TT_BLOCKS | BLACK_BRACKETS))
		putch_ascii(ch);
	else if (!putch_acs(ch))
		waddch(window, ch & ~0x100);
}

int printstr(const char *str)
{
	int x, y, w;
	int n = strlen(str);
	getyx(window, y, x);
	getmaxyx(window, y, w);
	if (x+n >= w)
		n = w-x-1;
	waddnstr(window, str, n);
	return n;
}

void printint(const char *fmt, int d)
{
	wprintw(window, fmt, d);
}

void printlong(const char *fmt, long d)
{
	wprintw(window, fmt, d);
}

int default_bgdot()
{
	return _XTERM ? BULLET : '.';
}
