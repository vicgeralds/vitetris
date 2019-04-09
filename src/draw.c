#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include "game/tetris.h"
#include "draw.h"
#include "textgfx/textgfx.h"
#include "version.h"
#include "hiscore.h"
#include "net/sock.h"	/* opponent_name */

char tetrom_colors[7] = {
	1,  /* I red */
	7,  /* J white */
	5,  /* L magenta */
	4,  /* O blue */
	2,  /* S green */
	3,  /* T yellow/brown */
	6   /* Z cyan */
};

int twoplayer_mode = 0;

static char board_bottom_color[2] = {BOARD_FRAME_COLOR, BOARD_FRAME_COLOR};

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

static void drawbl(int bl, int clr, int x, int y)
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

static void drawstr(const char *str, int n, int x, int y)
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

void draw_tetris_logo(int x, int y)
{
	int bl;
	drawbl(0x227, 1, x, y);
	drawbl(0x313, 4, x+7, y);
	drawbl(1, 4, x+8, y+1);
	drawbl(0x227, 6, x+12, y);
	drawbl(0x113, 7, x+19, y);
	drawbl(0x111, 2, x+24, y);
	bl = 0x326;
	if (is_outside_screen(x+33, 0))
		bl = 0x322;
	drawbl(bl, 3, x+27, y);
	setattr_normal();
}

void print_vitetris_ver(int x, int y)
{
	setcurs(x, y);
	setattr_bold();
	printstr(VITETRIS_VER);
	setattr_normal();
}

void draw_2p_menu_decor(int pl, int x, int y)
{
	x++;
	drawstr("\\l\\x", 0, x, y-1);
	y += 2;
	setcurs(x-1, y);
	putch(pl+'0');
	drawstr("P\\x\\2m", 0, x, y);
}

void draw_entergame()
{
	twoplayer_mode = game.mode & MODE_2P;
}

int board_x(int pl, int col)
{
	int x = 2*col;
	if (pl == 2)
		return x+35;
	if (twoplayer_mode)
		return x+1;
	return x+11;
}

void next_xy(int pl, int *x, int *y)
{
	if (_HEIGHT_24L)
		*x = board_x(pl, 3);
	else if (twoplayer_mode)
		*x = (pl==1) ? -9 : board_x(2, 11);
	else {
		*x = 1;
		*y = 15;
		return;
	}
	*y = 1;
}

static void drawboard(int pl)
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
	board_bottom_color[pl-1] = BOARD_FRAME_COLOR;
}

static int draw_vline(int x, int y, int h)
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

static void drawboard_frame_2p()
{
	int x = board_x(2, 10)+1;
	int y = _HEIGHT_24L ? 4 : 0;
	setwcurs(0, 0, y);
	if (!_MONOCHROME) {
		setcolorpair(BOARD_FRAME_COLOR);
		setattr_bold();
	}
	if (draw_vline(x, y, 20)) {
		x = board_x(1, 0)-2;
		setcurs(x, y);
		drawstr("x\\9x\\9x\\x", 0, x, y);
	}
}

static void drawpanel_labels(const char *first, int x)
{
	const char *s = first;
	int i;
	setcolorpair(PANEL_LABEL_COLOR);
	for (i = 1; i <= 9; i += 4) {
		setcurs(x, i);
		if (s[0]==' ')
			movefwd(1);
		else
			putch(' ');
		printstr(s);
		putch(' ');
		s = s==first ? "Level" : "Lines";
	}
}

static void printstat_1p()
{
	int secs;
	setattr_normal();
	if (!_WHITE_BG)
		setattr_bold();
	setcurs(1, 2);
	if (game.mode == MODE_1P_40L) {
		secs = player1.score/100;
		if (secs >= 600)
			printstr(" 9:99.9 ");
		else {
			printint(" %d:", secs/60);
			printint("%02d", secs%60);
			printint(".%d ", (player1.score%100)/10);
		}
	} else
		printlong(" %06ld ", player1.score % 1000000);
	setcurs(3, 6);
	printint(" %02d ", player1.level);
	setcurs(3, 10);
	printint(" %03d ", player1.lines);
}

static void printstat_2p(const struct player *p)
{
	int x;
	setattr_normal();
	if (!_WHITE_BG)
		setattr_bold();
	x = isplayer2(p) ? 7 : 2;
	setcurs(x, 2);
	printint(" %d ", p->score);
	if (x == 2)
		x--;
	setcurs(x, 6);
	printint(" %02d ", p->level);
	setcurs(x, 10);
	printint(" %02d ", p->lines);
}

static void drawpanel_bordercolor(int clr)
{
	if (_MONOCHROME)
		setattr_normal();
	else
		setcolorpair(clr | 16);
}

static void drawpanel_1p()
{
	const char *firstlabel = "Score";
	int h24 = _HEIGHT_24L;
	int clr = (player1.level % 6)+1;
	int i;
	if (game.mode == MODE_1P_40L)
		firstlabel = " Time";
	setwcurs(WIN_PANEL, 0, 0);
	setblockcolor(clr);
	printstr_acs("lqNu", 8);
	for (i = 0; i <= 8; i += 4)
		drawstr("\\xhNx\\3tqNu", 8, 0, i);
	if (h24) {
		setcurs(0, 12);
		putch(LOWLEFT);
		drawpanel_labels(firstlabel, 1);
	} else {
		setcurs(0, 13);
		printstr_acs("xhNx", 8);
		drawpanel_labels(firstlabel, 1);
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

static void drawpanel_2p()
{
	int i;
	setwcurs(WIN_PANEL, 0, 0);
	setblockcolor(4);
	printstr_acs("tqNu", 10);
	for (i = 0; i <= 8; i += 4)
		drawstr("\\xhNx\\3tqNu", 10, 0, i);
	drawpanel_labels("Wins", 3);
	printstat_2p(&player1);
	printstat_2p(&player2);
	drawpanel_bordercolor(4);
	drawstr("\\x Nx\\6x Nx", 10, 0, 12);
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
	int pos = 0;
	int i = 0;
	int j;
	if (term_width < 47 || !num_hiscores)
		return;
	setwcurs(WIN_TOP_SCORES, 0, 0);
	setcolorpair(MAGENTA_FG);
	printstr((game.mode == MODE_1P_40L) ? "Best Times" : "Top Scores");
	setattr_normal();
	while (i < 5 && i < num_hiscores) {
		newln(0);
		if (!pos && isbetterscore(i)) {
			hiscoreline();
			pos = 1;
			continue;
		}
		gethiscorestr(i, s);
		if (game.mode != MODE_1P_40L) {
			putch(i+'1');
			putch('.');
			for (j=1; s[j]==' '; j++)
				s[j] = '0';
		}
		printstr(s);
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
	if (game.state == GAME_PAUSED)
		clearboard_paused();
	else
		redrawboard(&player1, 19);
	drawpanel_1p();
	print_top_scores();
	print_tetr_stats();
	refreshwin(WIN_PANEL);
}

void drawgamescreen_2p()
{
	drawboard(1);
	drawboard(2);
	drawboard_frame_2p();
	refreshwin(0);
	redrawboard(&player1, 19);
	redrawboard(&player2, 19);
	drawpanel_2p();
	refreshwin(WIN_PANEL);
}

void print_game_message(int pl, const char *str, int bold)
{
	int x = board_x(pl, 4);
	int n = strlen(str);
	if (!(game.player[pl-1].rotation & ROT_LEFTHAND))
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
	if (game.mode & MODE_NET)
		print_game_message(2, opponent_name, 0);
	else
#endif
	if (twoplayer_mode)
		print_game_message(2, "PRESS KEY", 1);
	setcurs_end();
}

void drawnext(const struct player *p, const struct tetr *next)
{
	int win = WIN_NEXT+isplayer2(p);
	int bl;
	int x;
	if (twoplayer_mode && !_HEIGHT_24L && term_width < 76)
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
	int lefthand = p->rotation & ROT_LEFTHAND;
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
	if (twoplayer_mode)
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

static void drawredmeter(int x, int n)
{
	int i = 7;
	int c = ' ';
	while (i) {
		if (i <= n) {
			c = UPARROW;
			setblockcolor(1);
			n = 0;
		}
		newln(x);
		putch(c);
		putch(c);
		i--;
	}
	setattr_normal();
}

void upd_garbagemeter(const struct player *p, int n)
{
	char digits[3] = "  ";
	int x = isplayer2(p) ? 9  : 1;
	if (n) {
		board_bottom_color[isplayer2(p)] = 17;
		upd_dropmarker(p, 0);
		digits[0] = '0'+ n/10;
		digits[1] = '0'+ n;
	}
	setwcurs(WIN_PANEL, 0, 12);
	drawredmeter(x, n);
	if (n > 6)
		n = 6;
	setcurs(x, 19-n);
	printstr(digits);
	refreshwin(WIN_PANEL);
}

void show_winner(const struct player *p)
{
	int i = 1;
	while (1) {
		setwcurs(i, 4, 2);
		setattr_normal();
		setcolorpair(MAGENTA_FG);
		drawbox(4, 2, 12, 3, (char *)0);
		setcurs(6, 3);
		if (!p)
			printstr("  DRAW  ");
		else {
			printstr("YOU ");
			printstr(p==&game.player[i==2] ? "WIN!" : "LOSE");
		}
		refreshwin(i);
		if (i == 2)
			break;
		i = 2;
	}
	setcurs_end();
}

static void upddropmark(int pl, int a, int b, int c, int d)
{
	int clr = board_bottom_color[pl-1];
	if (term_height==20 || term_height==24 ||
	    pl==2 && game.mode & MODE_NET)
		return;
	if (game.state == GAME_RUNNING)
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
	board_bottom_color[i] = BOARD_FRAME_COLOR;
	setattr_normal();
	upddropmark(i+1, 0, 10, 0, 0);
}

void upd_screen(int i)
{
	refreshwin(i);
	setcurs_end();
}
