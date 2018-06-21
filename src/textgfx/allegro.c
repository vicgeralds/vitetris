#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>	/* for reading stdout.tmp etc. */
#include "../config.h"
#include <allegro.h>
#ifdef UNIX
#include <xalleg.h>	/* _xwin.application_name,
			   _xwin.application_class */
#endif
#include "textgfx.h"
#include "alleg.h"
#include "../options.h"
#include "../lang.h"
#include "../version.h"
#include "../input/keyboard.h"

#if defined ALLEGRO_WITH_XWINDOWS && defined ALLEGRO_USE_CONSTRUCTOR
#include "allegro_icon.h"
extern void *allegro_icon;
CONSTRUCTOR_FUNCTION(static void _set_allegro_icon());
static void _set_allegro_icon()
{
	allegro_icon = icon_xpm;
}
#endif

void blockstyle_from_option(const struct option *o);
int ibmgfx(int ch);

unsigned textgfx_flags = HEIGHT_24L;

char term_width = 80;
char term_height = 25;

static char curs_x = 0;
static char curs_y = 0;
char margin_x = 0;

int win_x = 0;
int win_y = 0;

BITMAP *virt_screen = NULL;
int refresh_needed = 0;
char blit_rect[4] = {0};

int close_button_pressed = 0;

static FONT *font8x16;
static int vgacolors[16];
static unsigned char bg_color;
static unsigned char fg_color;

#ifdef UNIX
static struct sigaction allegro_sigint_handler;
#endif

void gettermsize() {}
void settermwidth(int w) {}
void settermheight(int h) {}

void gettermoptions()
{
	struct option *o = getoptions("term");
	for (; o; o = o->next) {
		if (!strcmp(opt_key(o), "drawing")) {
			if (o->val.integ)
				textgfx_flags |= ASCII;
		} else if (!strcmp(opt_key(o), "color")) {
			if (!o->val.integ)
				textgfx_flags |= MONOCHROME;
		} else
			blockstyle_from_option(o);
	}
	reset_block_chars();
}

#ifdef UNIX
static void sigint_handler(int sig)
{
	textgfx_end();
	allegro_sigint_handler.sa_handler(sig);
}
#endif

static void load_pc8x16_font()
{
	char fname[256];
	int n;
	char *p;
	get_executable_name(fname, sizeof(fname));
	n = strlen(fname);
	do n--;
	while (fname[n] !='\\' && fname[n] !='/');
	fname[n] = '\0';
	append_filename(fname, fname, "pc8x16.fnt", sizeof(fname));
	font8x16 = load_font(fname, NULL, NULL);
	if (font8x16)
		return;
	p = getenv("ALLEGRO");
	if (p) {
		strncpy(fname, p, sizeof(fname));
		append_filename(fname, fname, "vitetris", sizeof(fname));
		set_allegro_resource_path(5, fname);
	}
#ifdef UNIX
	set_allegro_resource_path(4, "/usr/share/allegro/vitetris");
	set_allegro_resource_path(3, "/usr/share/allegro");
	set_allegro_resource_path(2, "/usr/local/share/allegro/vitetris");
	set_allegro_resource_path(1, "/usr/local/share/allegro");
#endif
	if (find_allegro_resource(fname, "pc8x16.fnt", 0,0,0,0,0,
							sizeof(fname)) == 0)
		font8x16 = load_font(fname, NULL, NULL);
}

static void close_btn()
{
	close_button_pressed = 1;
}

static void lost_focus()
{
	textgfx_flags |= LOST_FOCUS;
	clear_keybuf();
}

static void got_focus_back()
{
	textgfx_flags &= ~LOST_FOCUS;
	kb_flushinp();
	refresh_needed = 1;
	refreshscreen();
}

static int setgfxmode(int fullscreen)
{
	if (!fullscreen &&
	    set_gfx_mode(GFX_AUTODETECT_WINDOWED, 512, 400, 0, 0) == 0) {
		term_width = 64;
		set_display_switch_mode(SWITCH_BACKGROUND);
		set_display_switch_callback(SWITCH_OUT, lost_focus);
		return 1;
	}
	if (set_gfx_mode(GFX_AUTODETECT, 640, 480, 0, 0) == 0) {
		term_width = 80;
		set_display_switch_mode(SWITCH_PAUSE);
		return 1;
	}
	if (get_color_depth() != 8) {
		set_color_depth(8);
		return setgfxmode(fullscreen);
	}
	set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
	allegro_message("Unable to set any graphics mode:\n"
			"%s\n", allegro_error);
	return 0;
}

BITMAP *set_screen(int fullscreen)
{
	const unsigned char
	vgargb[16][3] = {
		  0,  0,  0,	/* 0 black */
		170,  0,  0,	/* 1 red */
		  0,170,  0, 	/* 2 green */
		170, 85,  0, 	/* 3 yellow */
		  0,  0,170,	/* 4 blue */
		170,  0,170,	/* 5 magenta */
		  0,170,170,	/* 6 cyan */
		170,170,170,	/* 7 white */
		 85, 85, 85,
		255, 85, 85,
		 85,255, 85,
		255,255, 85,
		 85, 85,255,
		255, 85,255,
		 85,255,255,
		255,255,255
	};
	const unsigned char *rgb;
	if (!setgfxmode(fullscreen))
		exit(1);
	clear_bitmap(screen);
	if (set_display_switch_callback(SWITCH_IN, got_focus_back) == -1)
		set_display_switch_mode(SWITCH_PAUSE);
	BITMAP *bmp = create_bitmap(8 * term_width, 400);
	clear_bitmap(bmp);
	set_clip_state(bmp, 0);
	set_clip_state(screen, 0);
	int i;
	for (i=0; i<16; i++) {
		rgb = &vgargb[i][0];
		vgacolors[i] = makecol(rgb[0], rgb[1], rgb[2]);
	}
	return bmp;
}

#if WIN32 && !ALLEGRO_USE_CONSOLE
static int printline(char *line)
{
	int n = strlen(line);
	int c = 0;
	if (line[n-1] == '\n')
		line[--n] = '\0';
	if (n > 64 && is_windowed_mode()) {
		c = line[64];
		line[64] = '\0';
	}
	printstr(line);
	if (!c || curs_y == 24)
		return 0;
	line[64] = c;
	curs_y++;
	printstr(line + 64);
	return 1;
}
#endif

void textgfx_init()
{
#ifdef UNIX
	strcpy(_xwin.application_name, "vitetris");
	strcpy(_xwin.application_class, "Vitetris");
#endif
	if (install_allegro(SYSTEM_AUTODETECT, &errno, NULL) != 0)
		exit(1);
#ifdef UNIX
	sigaction(SIGINT, NULL, &allegro_sigint_handler);
	signal(SIGINT, sigint_handler);
#endif
	load_pc8x16_font();
	set_window_title(VITETRIS_VER);
	set_close_button_callback(close_btn);
#ifndef UNIX
	/* Seems to cause seg fault later quite randomly on Linux  */
	int depth = desktop_color_depth();
	if (depth != 0)
		set_color_depth(depth);
#endif
	virt_screen = set_screen(getopt_int("", "fullscreen"));
	lang |= LATIN1;
	if (!font8x16) {
		font8x16 = font;
		textgfx_flags |= ASCII;
	}
	setattr_normal();
#if WIN32 && !ALLEGRO_USE_CONSOLE
	if (exists("stdout.tmp")) {
		FILE *fp;
		freopen("stdout2.tmp", "w", stdout);
		fp = fopen("stdout.tmp", "r");
		if (fp) {
			char line[80];
			int i;
			for (i=0; i < 25 && fgets(line, 80, fp); i++) {
				setcurs(0, i);
				i += printline(line);	
			}
			fclose(fp);
			if (i) {
				refreshscreen();
				if (!strncmp(line, "Press ", 6)) {
					install_keyboard();
					clear_keybuf();
					readkey();
					remove_keyboard();
				}
			}
		}
		freopen("stdout.tmp", "w", stdout);
		delete_file("stdout2.tmp");
	}
#endif
}

void textgfx_end()
{
	if (!virt_screen)
		return;
	destroy_bitmap(virt_screen);
	virt_screen = NULL;
	if (font8x16 != font)
		destroy_font(font8x16);
	allegro_exit();
}

void toggle_fullscreen()
{
	BITMAP *bmp = set_screen(is_windowed_mode());
	if (bmp->w == virt_screen->w)
		destroy_bitmap(bmp);
	else {
		if (bmp->w > virt_screen->w) {
			int h = 400;
			if (in_menu) {
				blit(virt_screen, screen, 0, 384, 0, 464,
							  512, 16);
				h = 384;
			}
			blit(virt_screen, bmp, 0, 0, 64, 0, 512, h);
			curs_x += 8;
		} else {
			blit(virt_screen, bmp, 64, 0, 0, 0, 512, 400);
			curs_x -= 8;
		}
		margin_x = getmargin_x();
		destroy_bitmap(virt_screen);
		virt_screen = bmp;
	}
	refresh_needed = 1;
	refreshscreen();
}

void setcurs(int x, int y)
{
	x += win_x + margin_x;
	y += win_y;
	curs_x = x;
	curs_y = y;
}

void movefwd(int n)
{
	curs_x += n;
}

void newln(int x)
{
	x += win_x + margin_x;
	curs_x = x;
	curs_y++;
}

void setcurs_end()
{
	curs_x = 0;
	curs_y = 24;
}

int is_outside_screen(int x, int y)
{
	return x + win_x + margin_x >= term_width ||
	       y + win_y >= 25;
}

void get_xy(int *x, int *y)
{
	*x = curs_x - win_x - margin_x;
	*y = curs_y - win_y;
}

void refreshscreen()
{
	int y = 0;
	if (!refresh_needed)
		return;
	if (!is_windowed_mode())
		y = 40;
	acquire_screen();
	blit(virt_screen, screen, 0, 0, 0, y, 8*term_width, 400);
	release_screen();
	refresh_needed = 0;
	memset(blit_rect, 0, 4);
}

static void update_blit_rect(char x, char y, char x2)
{
	if (x < blit_rect[0] || !blit_rect[0])
		blit_rect[0] = x;
	if (y < blit_rect[1] || !blit_rect[1])
		blit_rect[1] = y;
	if (x2 > blit_rect[2])
		blit_rect[2] = x2;
	if (y > blit_rect[3])
		blit_rect[3] = y;
}

void cleartoeol()
{
	int y = 16*curs_y;
	int maxx = term_width-1;
	rectfill(virt_screen, 8*curs_x, y, 8*maxx, y+15, 0);
	refresh_needed = 1;
	update_blit_rect(curs_x, curs_y, maxx);
}

void set_color_pair(int clr)
{
	int bg = 0;
	int bold = 1;
	if (_MONOCHROME) {
		if (clr == MAGENTA_FG)
			setattr_bold();
		return;
	}
	if (clr & 64) {
		bg = clr & 7;
		clr = clr>>3 & 7;
	} else
		switch (clr) {
		case MAGENTA_FG:
			clr = 5;
			break;
		case WHITE_ON_BLUE:
			clr = 7;
			bg = 4;
			break;
		case BOARD_BG_COLOR:
			clr = 4;
			bold = 0;
			break;
		case BOARD_FRAME_COLOR:
			clr = 4;
			bold = 0;
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
	if (textgfx_flags & BLACK_BRACKETS && bg == clr) {
		clr = 0;
		bold = 0;
	}
	if (!_TT_BLOCKS)
		bg_color = bg;
	if (bold)
		clr |= 8;
	fg_color = clr;
} 

void setattr_normal()
{
	bg_color = 0;
	fg_color = 7;
}

void setattr_standout()
{
	bg_color = fg_color;
	fg_color = 0;
}

void setattr_bold()
{
	if ((textgfx_flags & TT_MONO) != TT_MONO && fg_color < 8)
		fg_color |= 8;
}

void setattr_underline() {}

static int to_cp437(int ch)
{
	const char *latin1 = "ÅÄÖÜåäöü";
	const char *cp437 = "\x8F\x8E\x99\x9A\x86\x84\x94\x82";
	const char *p;
	p = strchr(latin1, ch);
	if (p)
		return (unsigned char) cp437[p-latin1];
	return '?';
}

static void text_out(const char *s, int n)
{
	int x = 8 *curs_x;
	int y = 16*curs_y;
	int bg = vgacolors[bg_color];
	int fg = vgacolors[fg_color];

	if (*s == '|' && n == 1) {
		rectfill(virt_screen, x, y, x+7, y+15, bg);
		rectfill(virt_screen, x+3, y, x+4, y+12, fg);
	} else {
		if (font == font8x16) {
			rectfill(virt_screen, x, y, x+(8*n)-1, y+15, bg);
			y += 4;
		}
		textout_ex(virt_screen, font8x16, s, x, y, fg, bg);
	}
	refresh_needed = 1;
	update_blit_rect(curs_x, curs_y, curs_x+n-1);
	curs_x += n;
}

void put_ch(int ch)
{
	char s[2] = "";
	int trans = 0;
	if (ch == '\b') {
		curs_x--;
		return;
	}
	if (ch < 0)
		ch = (unsigned char) ch;
	if (ch & 0x80)
		ch = to_cp437(ch);
	if (ch == TEXTURE2 && textgfx_flags & (TT_BLOCKS | BLACK_BRACKETS))
		ch = ' ';
	if (ch & 0x100) {
		if (_ASCII) {
			putch_ascii(ch);
			return;
		}
		ch = (unsigned char) ibmgfx(ch);
		if (ch < 32)
			trans = 32;
	}
	if (ch == 0xFA)
		trans = -0x80;
	else if (ch >= 0x80)
		trans = -0x60;
	if (trans) {
		transpose_font(font8x16, trans);
		s[0] = ch + trans;
		text_out(s, 1);
		transpose_font(font8x16, -trans);
	} else {
		s[0] = ch;
		text_out(s, 1);
	}
}

int printstr(const char *str)
{
	if (!str[0])
		return 0;
	while (str[0] && curs_x < term_width) {
		putch(str[0]);
		str++;
	}
	return 1;
}

void printint(const char *fmt, int d)
{
	char buf[80];
	int n = usprintf(buf, fmt, d);
	text_out(buf, n);
}

void printlong(const char *fmt, long d)
{
	char buf[80];
	int n = usprintf(buf, fmt, d);
	text_out(buf, n);
}

int default_bgdot()
{
	return BULLET;
}
