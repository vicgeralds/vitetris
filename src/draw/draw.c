#include <string.h>
#include <ctype.h>
#include <stdio.h>	/* sprintf */
#include "../game/tetris.h"
#include "draw.h"
#include "internal.h"
#include "../textgfx/textgfx.h"
#include "../hiscore.h"
#include "../netw/sock.h" /* opponent_name */

char tetrom_colors[7] = {
	1,  /* I red */
	7,  /* J white */
	5,  /* L magenta */
	4,  /* O blue */
	2,  /* S green */
	3,  /* T yellow/brown */
	6   /* Z cyan */
};

void drawbl(int bl, int clr, int x, int y)
{
	int c, i;
	setblockcolor(clr%10);
	i = clr<10 || textgfx_flags & (TT_BLOCKS | TT_BLOCKS_BG |
				       BLACK_BRACKETS);
	c = block_chars[i];
	while (bl) {
		setcurs(x, y);
		i = 1;
		do {
			if (!(bl & i))
				movefwd(2);
			else {
				putch(block_chars[0]);
				putch(c);
			}
		} while (bl & 16-(i <<= 1));
		bl >>= 4;
		y++;
	}
}

void next_xy(int pl, int *x, int *y)
{
	if (_HEIGHT_24L)
		*x = board_x(pl, 3);
	else if (TWOPLAYER_MODE)
		*x = (pl==1) ? -9 : board_x(2, 11);
	else {
		*x = 1;
		*y = 15;
		return;
	}
	*y = 1;
}

void drawstr(const char *str, int n, int x, int y)
{
	char s[8];
	const char *p;
	int m;
	while (p = strchr(str, '\\')) {
		m = p-str;
		memcpy(s, str, m);
		s[m] = '\0';
		str += m+1;
		if (isdigit(p[1])) {
			str++;
			m = p[1]-'0';
		} else
			m = 1;
		do {
			printstr_acs(s, n);
			setcurs(x, ++y);
		} while (--m);
	}
	printstr_acs(str, n);
}

void drawboard(int pl)
{
	int x = board_x(pl, 0);
	int i;
	if (_HEIGHT_24L) {
		i = x+5;
		setwcurs(0, i, 0);
		if (!_MONOCHROME) {
			setcolorpair(BOARD_FRAME_COLOR);
			setattr_bold();
		}
		drawstr("lqNk\\x Nx\\2|_N|", 8, i, 0);
		setcolorpair(BOARD_FRAME_COLOR);
		setcurs(x, 3);
		printstr("_____");
		movefwd(10);
		printstr("_____");
		i = 4;
	} else
		i = 0;
	x--;
	setwcurs(0, x, i);
	setcolorpair(BOARD_FRAME_COLOR);
	for (i = 0; i < 20; i++) {
		putch(VLINE);
		movefwd(20);
		putch(VLINE);
		if (i < 19)
			newln(x);
	}
	if (term_height > 20 && term_height != 24) {
		newln(x);
		printstr_acs("m*Nj", 20);
	} else {
		setcurs(0, term_height-2);
		newln(0);
	}
#ifdef TWOPLAYER
	board_bottom_color[pl-1] = BOARD_FRAME_COLOR;
#endif
}

void drawpanel_labels(const char *first, int x)
{
	const char *s = first;
	int i;
	setcolorpair(PANEL_LABEL_COLOR);
	for (i = 1; i <= 9; i += 4) {
		setcurs(x, i);
		putch(' ');
		printstr(s);
		putch(' ');
		s = s==first ? "Level" : "Lines";
	}
}

static void printstat_1p()
{
	setattr_normal();
	if (!_WHITE_BG)
		setattr_bold();
	setcurs(1, 2);
	printlong(" %06ld ", player1.score % 1000000);
	setcurs(3, 6);
	printint(" %02d ", player1.level);
	setcurs(3, 10);
	printint(" %03d ", player1.lines);
}

void drawpanel_bordercolor(int clr)
{
	if (_MONOCHROME)
		setattr_normal();
	else
		setcolorpair(clr | 16);
}

int draw_vline(int x, int y, int h)
{
	if (is_outside_screen(x+1, 0))
		return 0;
	movefwd(x);
	while (--h) {
		putch(VLINE);
		newln(x);
	}
	putch(VLINE);
	return 1;
}

static void drawpanel_1p()
{
	int h24 = _HEIGHT_24L;
	int clr = (player1.level % 6)+1;
	int i;
	setwcurs(WIN_PANEL, 0, 0);
	setblockcolor(clr);
	printstr_acs("lqNu", 8);
	for (i = 0; i <= 8; i += 4)
		drawstr("\\xhNx\\3tqNu", 8, 0, i);
	if (h24) {
		setcurs(0, 12);
		putch(LOWLEFT);
		drawpanel_labels("Score", 1);
	} else {
		setcurs(0, 13);
		printstr_acs("xhNx", 8);
		drawpanel_labels("Score", 1);
		setcurs(2, 13);
		printstr(" Next ");
	}
	printstat_1p();
	drawpanel_bordercolor(clr);
	if (h24)
		drawstr("\\x\\6x", 6, 9, 12);
	else {
		drawstr("\\x Nx\\3mqNu", 8, 0, 13);
		drawstr("\\x\\x", 0, 9, 17);
	}
	i = 0;
	if (h24)
		i = 4;
	setwcurs(0, 0, i);
	drawpanel_bordercolor(clr);
	draw_vline(board_x(1, 10)+1, i, 20);
}

static void hiscoreline()
{
	setcolorpair(RED_FG);
	putnchars('-', 9);
	setattr_normal();
}

static void print_top_scores()
{
	char s[8];
	const struct hiscore *hs = hiscores;
	int pos = 0;
	int i = 1;
	if (term_width < 47 || !hs[0].score)
		return;
	setwcurs(WIN_TOP_SCORES, 0, 0);
	setcolorpair(MAGENTA_FG);
	printstr("Top Scores");
	setattr_normal();
	while (hs->score && i <= 5) {
		newln(0);
		if (!pos && player1.score > hs->score) {
			hiscoreline();
			pos = 1;
			continue;
		}
		putch(i+'0');
		putch('.');
		if (hs->score < 1000000)
			putch(' ');
		sprintf(s, "%06ld", (long) hs->score);
		printstr(s);
		hs++;
		i++;
	}
	if (!pos) {
		newln(0);
		if (ishiscore())
			hiscoreline();
		else
			cleartoeol();
	}
	refreshwin(WIN_TOP_SCORES);
}

static void print_tetr_stats()
{
	const char letters[8] = "IJLOSTZ";
	int sum = 0;
	int i;
	if (term_width < 45)
		return;
	setwcurs(WIN_TETROM_STATS, 1, 0);
	for (i = 0; i < 7; i++) {
		setblockcolor(tetrom_colors[i]);
		putch(block_chars[0]);
		putch(letters[i]);
		putch(block_chars[1]);
		setattr_normal();
		printint(" %03d", tetr_stats[i]);
		newln(1);
		sum += tetr_stats[i];
	}
	printstr("  -----");
	newln(0);
	printint("Sum %04d", sum);
	refreshwin(WIN_TETROM_STATS);
}

void drawgamescreen_1p()
{
	drawboard(1);
	refreshwin(0);
	if (game_paused)
		clearboard_paused();
	else
		redrawboard(&player1, 19);
	drawpanel_1p();
	print_top_scores();
	print_tetr_stats();
	refreshwin(WIN_PANEL);
}

void print_game_message(int pl, const char *str, int bold)
{
	int x = board_x(pl, 4);
	int n = strlen(str);
	if (!(game->player[pl-1].rotationsys & ROT_LEFTHAND))
		x--;
	while (n >= 6) {
		x--;
		n -= 2;
	}
	setwcurs(0, x, _HEIGHT_24L ? 7 : 3);
	setattr_normal();
	if (bold)
		setcolorpair(MAGENTA_FG);
	printstr(str);
	refreshwin(0);
}

void print_press_key()
{
	print_game_message(1, "PRESS KEY", 1);
#ifdef SOCKET
	if (game->mode & MODE_NETWORK)
		print_game_message(2, opponent_name, 0);
	else
#endif
	if (TWOPLAYER_MODE)
		print_game_message(2, "PRESS KEY", 1);
	setcurs_end();
}

void drawnext(const struct player *p, const struct tetr *next)
{
	int win = WIN_NEXT+isplayer2(p);
	int bl;
	int x;
	if (TWOPLAYER_MODE && !_HEIGHT_24L && term_width < 76)
		return;
	clearwin(win);
	if (next && (bl = next->blocks)) {
		x = (bl!=TETR_I && bl!=TETR_O);
		while (!(bl & 15))
			bl >>= 4;
		setcurs(x, 0);
		drawbl(bl, next->color+10, x, 0);
	}
	upd_screen(win);
}

static void drawboard_bg(const struct player *p, int col)
{
	int lefthand = p->rotationsys & ROT_LEFTHAND;
	putch( (lefthand && col%2) ? bgdot : ' ');
	putch(!(lefthand || col%2) ? bgdot : ' ');
}

void drawblocks(const struct player *p, int bl, int x, int y,
		enum drawmode mode)
{
	int i, j;
	x *= 2;
	while (y < 0) {
		bl >>= 4;
		y++;
	}
	setwcurs(1+isplayer2(p), x, y);
	if (mode != CLEAR_BLOCKS) {
		i = p->piece.color;
		if (mode == DRAW_PIECE)
			i += 10;
		drawbl(bl, i, x, y);
	} else {
		setblockcolor(0);
		while (1) {
			i = 1;
			j = x/2;
			do {
				if (bl & i)
					drawboard_bg(p, j);
				else
					movefwd(2);
				j++;
			} while (bl & 16-(i <<= 1));
			if (!(bl >>= 4))
				break;
			y++;
			setcurs(x, y);
		}
	}
}

void drawrow(const struct player *p, int y)
{
	uint_least32_t bl = p->board[y];
	int clr = -1;
	int i = 0;
	setwcurs(1+isplayer2(p), 0, y);
	do {
		if (clr != (bl & 7)) {
			clr = bl & 7;
			setblockcolor(clr);
		}
		if (clr) {
			putch(block_chars[0]);
			putch(block_chars[1]);
		} else
			drawboard_bg(p, i);
		bl >>= 3;
	} while (++i < 10);
}

void redrawboard(const struct player *plr, int y)
{
	while (y >= 0) {
		drawrow(plr, y);
		y--;
	}
	if (plr->piece.blocks)
		drawpiece(plr, plr->piece.blocks, plr->piece.x, plr->piece.y);
	refreshwin(1+isplayer2(plr));
}

void clearboard_paused()
{
	int x, y;
	int i, j;
	getwin_xy(1, &x, &y);
	setwcurs(0, x, y);
	setblockcolor(0);
	for (i = 0; i < 20; i++) {
		setcurs(x, y+i);
		for (j = 0; j < 10; j++)
			drawboard_bg(&player1, j);
	}
	print_game_message(1, "-- PAUSE --", 1);
}

void upd_stat(const struct player *p, int levelup)
{
	setwcurs(WIN_PANEL, 1, 2);
	if (TWOPLAYER_MODE)
		printstat_2p(p);
	else {
		if (levelup)
			drawpanel_1p();
		else
			printstat_1p();
		print_top_scores();
		print_tetr_stats();
	}
	refreshwin(WIN_PANEL);
}

static void upddropmark(int pl, int a, int b, int c, int d)
{
#ifdef TWOPLAYER
	int clr = board_bottom_color[pl-1];
#else
	int clr = BOARD_FRAME_COLOR;
#endif
	if (term_height==20 || term_height==24 ||
	    pl==2 && game->mode & MODE_NETWORK)
		return;
	if (game_running)
		refreshwin(pl);
	setwcurs(0, board_x(pl, a), _HEIGHT_24L ? 24 : 20);
	setcolorpair(clr);
	putnchars('*', 2*(b-a));
	setattr_normal();
	putnchars('=', 2*c);
	setcolorpair(clr);
	putnchars('*', 2*d);
}

void upd_dropmarker(const struct player *p, int mv)
{
	int pl = 1+isplayer2(p);
	int blocks = p->piece.blocks;
	int col = p->piece.x + !(blocks & 0x1111);
	int w = 2;
	switch (blocks) {
	case 0: col=0; w=0; break;
	case TETR_I:  w = 4; break;
	case TETR_I2: w = 1; break;
	default:
		w += blocks & 0x1111 && blocks & 0x4444;
	}
	switch (mv) {
	case -1:
		upddropmark(pl, col-1, col-1, w, 1);
		break;
	case 0:
		upddropmark(pl, 0, col, w, 10-col-w);
		break;
	case 1:
		upddropmark(pl, col, col+1, w, 0);
	}
}

void hide_dropmarker(const struct player *p)
{
	int i = isplayer2(p);
#ifdef TWOPLAYER
	board_bottom_color[i] = BOARD_FRAME_COLOR;
#endif
	setattr_normal();
	upddropmark(i+1, 0, 10, 0, 0);
}

void drawbox(int x, int y, int w, int h, const char *title)
{
	int i;
	setcurs(x, y);
	if (!title)
		printstr_acs("lqNk", w-2);
	else {
		setattr_normal();
		setattr_standout();
		i = w-strlen(title)-4;
		printstr_acs("lqN ", i/2);
		printstr(title);
		printstr_acs(" qNk", i-i/2);
		setattr_normal();
	}
	for (i = 2; i < h; i++) {
		newln(x);
		printstr_acs("x Nx", w-2);
	}
	newln(x);
	printstr_acs("mqNj", w-2);
}

void clearbox(int x, int y, int w, int h)
{
	setattr_normal();
	setcurs(x, y);
	while (1) {
		if (!w)
			cleartoeol();
		else
			putnchars(' ', w);
		h--;
		if (!h)
			break;
		newln(x);
	}
}

void upd_screen(int i)
{
	refreshwin(i);
	setcurs_end();
}
