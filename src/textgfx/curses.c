#include <string.h>
#include <curses.h>
#include "textgfx.h"
#include "../game/tetris.h"
#include "../draw.h"

#ifdef NCURSES_VERSION
#define COLOR_1_6(i) (i+1)
#define COLOR_DEFAULT_BG -1
#else
#define COLOR_1_6(i) colors1_6[i]
#define COLOR_DEFAULT_BG COLOR_BLACK

static const short colors1_6[6] = {
	COLOR_RED,  COLOR_GREEN,   COLOR_YELLOW,
	COLOR_BLUE, COLOR_MAGENTA, COLOR_CYAN
};
#endif

unsigned textgfx_flags = 0;

WINDOW *window = NULL;
WINDOW *wins[6] = {NULL};

static int margin_x;

static chtype acs_chars[17];

static void delwins();
static void createwin(int i, int w, int h);

static void initpair(short pair, short f, short b)
{
	if (_TT_BLOCKS || f==b && textgfx_flags & BLACK_BRACKETS)
		b = COLOR_DEFAULT_BG;
	init_pair(pair, f, b);
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

void set_input_mode();

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

	if (!_XTERM)
		default_bgdot = '.';
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

void txtg_entermenu()
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

void txtg_entergame()
{
	delwins();
	margin_x = getmargin_x();
	createwin(1, 20, 20);
	if (!twoplayer_mode) {
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
	print_ver_author();
}

static void delwins()
{
	int i;
	for (i = 0; i < 6; i++)
		if (wins[i]) {
			delwin(wins[i]);
			wins[i] = NULL;
		}
}

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

static void createwin(int i, int w, int h)
{
	int x, y;
	getwin_xy(i, &x, &y);
	i = winindex(i);
	wins[i] = newwin(h, w, y, x+margin_x);
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
		if (game.state == GAME_PAUSED && y <= 4)
			clearboard_paused_fix();
	}
	setcurs(x, y);
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

void refreshwin(int i)
{
	WINDOW *win;
	if (i == -1)
		win = window;
	else
		win = wins[winindex(i)];
	if ((i==1 || i==2) && game.state != GAME_RUNNING) {
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

static int set_panel_label_color()
{
	short f = COLOR_YELLOW;
	short b;
	if (twoplayer_mode)
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

static void set_color_pair(int pair)
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

void setcolorpair(int pair)
{
	if (pair == PANEL_LABEL_COLOR)
		pair = set_panel_label_color();
	set_color_pair(pair);
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
