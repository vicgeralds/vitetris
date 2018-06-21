#include <stdio.h>
#include <string.h>
#include "textgfx.h"
#include "ansivt.h"
#include "../input/termin.h"

#ifdef NO_BLOCKSTYLES
#undef TT_BLOCKS
#undef BLACK_BRACKETS
#define TT_BLOCKS 0
#define BLACK_BRACKETS 0
#endif

unsigned textgfx_flags = ASCII;

char curs_x = 0;
char curs_y = 0;
char margin_x = 0;
char menuheight = 1;

int win_x = 0;
int win_y = 0;

void textgfx_init()
{
	set_input_mode();
#ifdef UNIX
	printf("\033[?25l");	/* hide cursor */
#endif
}

void textgfx_end()
{
	if (textgfx_flags == 0xFFF)
		return;
	setcurs_end();
	putch(' ');	/* switch back from acs */
	restore_input_mode();
	printf(
#ifdef UNIX
	"\033[?25h"
#endif
	"\033[m\n");
	textgfx_flags = 0xFFF;
}

static void set_curs(int x, int y)
{
	if (x != curs_x) {
		if (x > curs_x)
			printf("\033[%dC", x-curs_x);
		else
			printf("\033[%dD", curs_x-x);
		curs_x = x;
	}
	if (y != curs_y) {
		if (y < curs_y)
			printf("\033[%dA", curs_y-y);
		else
			printf("\033[%dB", y-curs_y);
		curs_y = y;
	}
}

void setcurs(int x, int y)
{
	x += win_x + margin_x;
	y += win_y;
	set_curs(x, y);
}

void movefwd(int n)
{
	printf("\033[%dC", n);
	curs_x += n;
}

void newln(int x)
{
	putchar('\n');
	x += win_x + margin_x;
	if (x)
		printf("\033[%dC", x);
#ifndef NO_MENU
	if (menuheight && curs_x > margin_x+1 && curs_y+1 >= menuheight)
		menuheight = curs_y+1;
#endif
	curs_x = x;
	curs_y++;
}

void setcurs_end()
{
	int y;
#ifdef NO_MENU
	if (0)
#else
	if (menuheight)
#endif
		y = menuheight;
	else {
		y = term_height-1;
		if (y > 20 && y < 23)
			y = 20;
	}
	set_curs(0, y);
	fflush(stdout);
}

int is_outside_screen(int x, int y)
{
	return x + win_x + margin_x >= term_width ||
	       y + win_y >= term_height;
}

void get_xy(int *x, int *y)
{
	*x = curs_x - win_x - margin_x;
	*y = curs_y - win_y;
}

void refreshscreen()
{
	fflush(stdout);
}

void cleartoeol()
{
	printf("\033[K");
#ifndef NO_MENU
	if (curs_x <= margin_x+2 && curs_y < menuheight && curs_y > 4)
		menuheight = curs_y;
#endif
}

void set_ansi_color(int bg, int fg, char bold)
{
	if (textgfx_flags & BLACK_BRACKETS) {
		if (bg == fg) {
			set_ansi_color(-1, fg, '0');
			printf("\033[7m");
			return;
		} 
		if (bg >= 0)
			printf("\033[m");
	}
	if (!_TT_BLOCKS) {
		if (bg >= 0)
			printf("\033[4%cm", bg+'0');
		else
			printf("\033[m");
	}
	printf("\033[%c;3%cm", bold, fg+'0');
}

void set_color_pair(int clr)
{
	int bg = -1;
	char bold = '1';
	if (_MONOCHROME) {
		if (clr == MAGENTA_FG)
			setattr_bold();
		return;
	}
	switch (clr) {
	case 7:
		if (_WHITE_BG)
			clr = 0;
		bg = clr;
		break;
	case 0x17:
		printf("\033[m\033[1m");
		return;
	case MAGENTA_FG:
		clr = 5;
		break;
	case WHITE_ON_BLUE:
		clr = 7;
		bg = 4;
		break;
	case BOARD_BG_COLOR:
		clr = _WHITE_BG ? 6 : 4;
		bold = '0';
		break;
	case BOARD_FRAME_COLOR:
		clr = 4;
		bold = '0';
		break;
	case RED_FG:
		clr = 1;
		break;
	case YELLOW_ON_BLUE:
		clr = 3;
		bg = 4;
		break;
	case YELLOW_ON_GREEN:
		clr = 3;
		bg = 2;
		break;
	default:
		if (clr & 16)
			clr &= 7;
		else
			bg = clr;
	}
	set_ansi_color(bg, clr, bold);
}

void setattr_normal()
{
	printf("\033[m");
}

void setattr_standout()
{
	printf("\033[7m");
}

void setattr_bold()
{
#ifndef NO_BLOCKSTYLES
	if ((textgfx_flags & TT_MONO) != TT_MONO)
#endif
		printf("\033[1m");
}

void setattr_underline()
{
	printf("\033[4m");
}

void put_ch(int ch)
{
	static int acs;
	if (ch < 0)
		ch = (unsigned char) ch;
	if (ch == TEXTURE2 && textgfx_flags & (TT_BLOCKS | BLACK_BRACKETS))
		ch = ' '|0x100;
	if (ch & 0x100) {
		if (_ASCII) {
			putch_ascii(ch);
			return;
		}
#if IBMGRAPHICS
		ch = ibmgfx(ch);
#else
		if (ch == (' '|0x100) || ch == TEXTURE2 && !_LINUX_TERM)
			ch = ' ';
		else if (!acs) {
			printf("\033(0");
			acs = 1;
		}
		ch &= ~0x100;
	} else if (acs) {
		printf("\033(B");
		acs = 0;
#endif
	}
	putchar(ch);
	curs_x += ch=='\b' ? -1 : 1;
}

int printstr(const char *str)
{
	int len = strlen(str);
	int i;
	if (!len)
		return 0;
	if (curs_x+len >= term_width) {
		len = term_width-curs_x-1;
		if (len < 1)
			return 0;
	}
	putch(str[0]);
	for (i = 1; i < len; i++) {
		putchar(str[i]);
		curs_x++;
	}
	return 1;
}

void printint(const char *fmt, int d)
{
	putch(' ');
	putch('\b');
	curs_x += printf(fmt, d);
}

void printlong(const char *fmt, long d)
{
	putch(' ');
	putch('\b');
	curs_x += printf(fmt, d);
}

int default_bgdot()
{
	return BULLET;
}
