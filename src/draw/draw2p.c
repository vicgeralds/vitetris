#undef TWOPLAYER
#define TWOPLAYER 1
#include "../game/tetris.h"
#include "draw.h"
#include "internal.h"
#include "../textgfx/textgfx.h"

char board_bottom_color[2] = {BOARD_FRAME_COLOR, BOARD_FRAME_COLOR};

int board_x(int pl, int col)
{
	int x = 2*col;
	if (pl == 2)
		return x+35;
	if (TWOPLAYER_MODE)
		return x+1;
	return x+11;
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

void printstat_2p(const struct player *p)
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
			printstr(p==game->player+(i==2) ? "WIN!" : "LOSE");
		}
		refreshwin(i);
		if (i == 2)
			break;
		i = 2;
	}
	setcurs_end();
}
