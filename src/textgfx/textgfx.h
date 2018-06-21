/*
 * textgfx flags
 */
#define ASCII 1
#define WHITE_BG 2
#define MONOCHROME 4
#define HEIGHT_24L 8

/* block styles */
#define BLACK_BRACKETS 16
#define TT_BLOCKS 32
#define TT_BLOCKS_BG 64

#define XTERM 0x80
#define LINUX_TERM 0x100
#define GNOME_TERM 0x200
#define CYGWIN 0x400
#define TERM_RESIZED 0x800
#define LOST_FOCUS 0x1000

extern unsigned textgfx_flags;

#define _ASCII	    (textgfx_flags & ASCII)
#define _WHITE_BG   (textgfx_flags & WHITE_BG)
#define _MONOCHROME (textgfx_flags & MONOCHROME)
#define _HEIGHT_24L (textgfx_flags & HEIGHT_24L)
#define _TT_BLOCKS  (textgfx_flags & TT_BLOCKS)
#define _TT_BLOCKS_BG (textgfx_flags & TT_BLOCKS_BG)
#define TT_MONO (TT_BLOCKS | MONOCHROME)
#define _XTERM (textgfx_flags & XTERM)
#define _LINUX_TERM (textgfx_flags & LINUX_TERM)

extern char term_width;
extern char term_height;

extern short block_chars[2];
extern short bgdot;

int default_bgdot();
void reset_block_chars();

void gettermsize();
void settermwidth(int cols);
void settermheight(int lines);
void gettermoptions();

void textgfx_init();
void textgfx_end();

#ifdef NO_MENU
#define in_menu 0
#define textgfx_entermenu()
#else
extern int in_menu;
void textgfx_entermenu();
#endif
void textgfx_entergame();

/* Windows
 0 - standard screen
 1 - board 1
 2 - board 2 */
#define WIN_NEXT 3
/* WIN_NEXT+1 - player 2's next */
#define WIN_PANEL 5
#define WIN_TETROM_STATS 6
#define WIN_TOP_SCORES 7

int getmargin_x();
void getwin_xy(int win, int *x, int *y);

void setcurs(int x, int y);
void setwcurs(int win, int x, int y);
void movefwd(int n);
void newln(int x);
void setcurs_end();
int is_outside_screen(int x, int y);
void get_xy(int *x, int *y);

void refreshscreen();

/* refreshwin(-1) refreshes the current window */
void refreshwin(int win);

void clearwin(int win);
void cleartoeol();

/* Color pairs.
 * 1-7 are used for blocks.
 * 0x11-0x17 are block colors without bg. */
#define MAGENTA_FG 8
#define WHITE_ON_BLUE 9
#define BOARD_BG_COLOR 10
#define BOARD_FRAME_COLOR 11
#define PANEL_LABEL_COLOR 12
#define RED_FG 13
#define YELLOW_ON_GREEN 14
#define YELLOW_ON_BLUE 0x18

void setcolorpair(int pair);

/* inner func - doesn't set PANEL_LABEL_COLOR which depends on tetris level */
void set_color_pair(int pair);

/* Set color pair and block chars based on block style.
 * clr = 1-7 or 0 for board bg. */
void setblockcolor(int clr);

void setattr_normal();
void setattr_standout();
void setattr_bold();
void setattr_underline();

/* Line drawing characters - used with putch */

#define UPLEFT	 ('l' | 0x100)
#define LOWLEFT  ('m' | 0x100)
#define UPRIGHT	 ('k' | 0x100)
#define LOWRIGHT ('j' | 0x100)
#define CROSSLINES ('n' | 0x100)
#define HLINE	 ('q' | 0x100)
#define VLINE	 ('x' | 0x100)
#define LEFT_T	 ('t' | 0x100)
#define RIGHT_T	 ('u' | 0x100)
#define BOTTOM_T ('v' | 0x100)
#define TOP_T	 ('w' | 0x100)
#define TEXTURE1 ('a' | 0x100)	/* ACS_CKBOARD */
#define TEXTURE2 ('h' | 0x100)	/* ACS_BOARD or blank */
#define BULLET	 ('~' | 0x100)
#define UPARROW	 ('-' | 0x100)

/* putch conflicts with DOS conio */
void put_ch(int ch);
#ifndef conio_h
#define putch(ch) put_ch(ch)
#endif
int printstr(const char *str);
void printint(const char *fmt, int d);
void printlong(const char *fmt, long d);

/* takes a line drawing character and prints ascii */
void putch_ascii(int ch);

int putnchars(int ch, int n);

/* prints lowercase letters in str using alternate character set,
 * and repeats a character followed by 'N' n times */
void printstr_acs(const char *str, int n);

int isprintable(int c);

void enable_term_resizing();
void upd_termresize();
