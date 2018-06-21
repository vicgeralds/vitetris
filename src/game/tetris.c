#include <stdlib.h>
#include <string.h>
#include "tetris.h"
#include "../timer.h"
#include "../input/input.h"
#include "../draw/draw.h"

struct game *game = NULL;
char clearedlines[4];

#define RETURN_ON_INPUT 4

/* process single-player input */
static int processinput(int tm, int flags);

int randnum(int n)
{
	return (int) (n*(rand()/(RAND_MAX+1.0)));
}

void gettetrom(struct tetr *t, int i)
{
	t->x = 3;
	t->y = -3;
	switch (i) {
	case 0:
		t->y = -4;
		t->blocks = TETR_I;
		break;
	case 1:
		t->blocks = TETR_J;
		break;
	case 2:
		t->blocks = TETR_L;
		break;
	case 3:
		t->y = -2;
		t->blocks = TETR_O;
		break;
	case 4:
		t->blocks = TETR_S;
		break;
	case 5:
		t->blocks = TETR_T;
		break;
	case 6:
		t->blocks = TETR_Z;
		break;
	default:
		t->blocks = 0;
		return;
	}
	t->color = tetrom_colors[i];
}

int hitbtm(struct tetr *p, struct player *plr)
{
	uint_least32_t *board;
	int i, mask;
	int x, y;

	i = 12;
	while (!(p->blocks >> i))
		i -= 4;
	if ((y = p->y+i/4) == 19)
		return 1;
	board = plr->board;
	if (y < -1 || !board[y+1])
		return 0;
	mask = 15;
	while (1) {
		if (p->blocks & mask<<i) {
			for (x = 0; x < 4; x++)
				if (1<<x & mask && p->blocks & 1<<i+x &&
				    board[y+1]>>3*(p->x+x) & 7)
					return 1;
			mask ^= p->blocks>>i & mask;
		}
		if (!i || y < 0)
			break;
		i -= 4;
		y--;
	}
	return 0;
}

static void levelup(struct player *p)
{
	p->level++;
	if (!p->level)
		p->falltime = 799;
	else if (p->level < 9)
		p->falltime -= 83;
	else if (p->level == 9)
		p->falltime = 100;
	else if (p->level == 10)
		p->falltime = 83;
	else if (p->level < 20)
		p->falltime -= 5 + p->level%2;
	else if (p->level < 30)
		p->falltime -= 2;
}

static void upd_score_level(struct player *p, int n)
{
	int lines;
	if (!(game->mode & MODE_2PLAYER)) {
		switch (n) {
		case 1:
			p->score += 40*(p->level+1);
			break;
		case 2:
			p->score += 100*(p->level+1);
			break;
		case 3:
			p->score += 300*(p->level+1);
			break;
		case 4:
			p->score += 1200*(p->level+1);
		}
	}
	if (game->mode & MODE_BTYPE) {
		p->lines -= n;
		lines = p->lineslimit - p->lines;
		if (p->lines <= 0) {
			p->lines = 0;
			p->piece.y = -1;
		}
	} else {
		p->lines += n;
		lines = p->lines;
	}
	if (p->level == p->startlevel) {
		if (lines >= 10*p->startlevel+10 || lines >= 100)
			levelup(p);
	} else if (lines/10 != (lines-n)/10)
		levelup(p);
}

static void clearlines(struct player *p, int n)
{
	uint_least32_t *board = p->board;
	unsigned char lines[4];
	int i;
	upd_score_level(p, n);
	memcpy(lines, clearedlines, 4);
	for (i = 0; i < n; i++)
		board[lines[i]-1] = 0;
	while (n--) {
		for (i = lines[n]-1; i > 0; i--) {
			board[i] = board[i-1];
			if (!n && !board[i])
				break;
		}
		if (board[0])
			board[0] = 0;
		for (i = n-1; i >= 0; i--)
			lines[i]++;
	}
}

static void draw_lineclear_anim(int n)
{
	int x, i;
	int t = gettm(0);
	int tm;
	for (x = 4; x >= 0; x--) {
		for (i = 0; i < n; i++) {
			clearblocks(&player1, 1, x, clearedlines[i]-1);
			clearblocks(&player1, 1, 9-x, clearedlines[i]-1);
		}
		tm = CLEAR_DELAY/5;
		tm = (5-x)*tm-gettm(t)+t;
		processinput(tm, DISCARD_MOVES | NO_PAUSE);
	}
	redrawboard(&player1, clearedlines[n-1]-1);
}

static int lockdelay()
{
	struct tetr *p = &player1.piece;
	int t = gettm(0);
	int d = 0;
	int x1, x2;
	int b1, b2;

	x1 = x2 = p->x;
	b1 = b2 = p->blocks;
	while (d < LOCK_DELAY) {
		switch (processinput(LOCK_DELAY-d, RETURN_ON_INPUT)) {
		case -2:
			return -2;
		case 0:
			return 0;
		}
		if (!hitbtm(p, &player1))
			return 1;
		if (p->x != x2) {
			if (p->x == x1)
				break;
			x1 = x2;
			x2 = p->x;
		} else if (p->blocks != b2) {
			if (p->blocks == b1)
				break;
			b2 = p->blocks;
		} else
			d = -1;
		if (d == -1)
			d = gettm(t)-t;
		else {
			d = gettm(t)-t-LOCK_DELAY+d;
			if (d < 0)
				d = 0;
			else if (d > LOCK_DELAY-10)
				d = LOCK_DELAY-10;
			t = gettm(0)-d;
		}
	}
	return 0;
}

void lockpiece(struct player *plr)
{
	struct tetr *p = &plr->piece;
	int x, y;
	int n = 0;

	if (!p->blocks)
		return;
	drawblocks(plr, p->blocks, p->x, p->y, DRAW_BLOCKS);
	hide_dropmarker(plr);
	while (!(p->blocks & 15)) {
		p->blocks >>= 4;
		p->y++;
	}
	if (p->y < 0) {
		p->blocks = 0;
		return;
	}
	memset(clearedlines, 0, 4);
	y = p->y;
	do {
		x = 0;
		while (!(p->blocks & 1<<x))
			x++;
		do plr->board[y] |= (uint_least32_t) p->color << 3*(p->x+x);
		while (p->blocks & 1<<++x);

		for (x = 0; x < 10; x++)
			if (!(plr->board[y]>>3*x & 7))
				break;
		if (x == 10)
			clearedlines[n++] = y+1;
		y++;
	} while (p->blocks >>= 4);
	x = plr->level;
	if (n)
		clearlines(plr, n);
	upd_stat(plr, x != plr->level);
	if (n && !(game->mode & MODE_2PLAYER))
		draw_lineclear_anim(n);
}

static int can_moveright(struct tetr *p, uint_least32_t *board)
{
	int bl = p->blocks;
	int i;
	int x, y;

	if (i = (bl != TETR_I))
		i += !(bl & 0x444);
	if (p->x-i > 5)
		return 0;
	y = p->y;
	while (y < 0 || !(bl & 15)) {
		bl >>= 4;
		y++;
	}
	x = 3-i;
	i = 0;
	while (1) {
		while (!(bl & 1<<i+x))
			x--;
		if (board[y] && board[y]>>3*(p->x+x+1) & 7)
			return 0;
		if (x)
			bl &= ~(((1<<x)-1) << i);
		if (!(bl >> (i += 4)))
			break;
		x = 2-!(bl & 0x444);
		y++;
	}
	return bl;
}

void moveright(struct player *plr)
{
	struct tetr *p = &plr->piece;
	int bl = can_moveright(p, plr->board);
	int y = p->y;
	int i = 0;
	if (!bl) {
		if (plr->falltime < LOCK_DELAY) {
			plr->mvright_tm = gettm(0);
			plr->mvleft_tm = 0;
		}
		return;
	}
	while (y < 0 || !(p->blocks & 15<<i)) {
		y++;
		i += 4;
	}
	drawpiece(plr, bl, p->x+1, y);
	bl = p->blocks >> i;
	do {
		i = 1;
		while (!(bl & i))
			i <<= 1;
		clearblocks(plr, bl & i, p->x, y);
		y++;
	} while (bl >>= 4);
	upd_dropmarker(plr, 1);
	p->x++;
}

static int can_moveleft(struct tetr *p, uint_least32_t *board)
{
	int bl;
	int i;
	int x, y;

	if (!(p->x + !(p->blocks & 0x111)))
		return 0;
	bl = p->blocks;
	y = p->y;
	while (y < 0 || !(bl & 15)) {
		bl >>= 4;
		y++;
	}
	i = 0;
	while (1) {
		x = 0;
		while (!(bl & 1<<i+x))
			x++;
		if (board[y] && board[y]>>3*(p->x+x-1) & 7)
			return 0;
		if (x < 3)
			bl &= ~(16-(1<<x+1) << i);
		if (!(bl >> (i += 4)))
			break;
		y++;
	} 
	return bl;
}

void moveleft(struct player *plr)
{
	struct tetr *p = &plr->piece;
	int bl = can_moveleft(p, plr->board);
	int y = p->y;
	int i = 0;
	if (!bl) {
		if (plr->falltime < LOCK_DELAY) {
			plr->mvleft_tm = gettm(0);
			plr->mvright_tm = 0;
		}
		return;
	}
	while (y < 0 || !(p->blocks & 15<<i)) {
		y++;
		i += 4;
	}
	drawpiece(plr, bl, p->x-1, y);
	bl = p->blocks >> i;
	do {
		i = 8;
		while (!(bl & i))
			i >>= 1;
		clearblocks(plr, bl & i, p->x, y);
		y++;
	} while (bl >>= 4);
	upd_dropmarker(plr, -1);
	p->x--;
}

static void drop_score()
{
	if (!(game->mode & MODE_2PLAYER))
		player1.score++;
}

int movedown(struct player *plr, int drop)
{
	struct tetr *p = &plr->piece;
	int bl;
	int i, mask;

	if (hitbtm(p, plr))
		return 0;
	if (drop)
		drop_score();
	bl = 0;
	mask = 15;
	for (i = 12; i >= 0; i -= 4)
		if (p->blocks & mask<<i) {
			bl |= p->blocks & mask<<i;
			mask ^= bl>>i & mask;
		}
	drawpiece(plr, bl, p->x, p->y+1);
	bl = 0;
	mask = 15;
	for (i = 0; i <= 12; i += 4)
		if (p->blocks & mask<<i) {
			bl |= p->blocks & mask<<i;
			mask ^= bl>>i & mask;
		}
	clearblocks(plr, bl, p->x, p->y);
	p->y++;
	if (plr->falltime >= LOCK_DELAY)
		goto out;
	if (!drop && p->y >= 0) {
		if (plr->mvright_tm && LOCK_DELAY - plr->falltime >=
				gettm(plr->mvright_tm) - plr->mvright_tm)
			moveright(plr);
		else if (plr->mvleft_tm && LOCK_DELAY - plr->falltime >=
				gettm(plr->mvleft_tm) - plr->mvleft_tm)
			moveleft(plr);
	}
	plr->mvright_tm = 0;
	plr->mvleft_tm = 0;
out:	upd_screen(1+(plr > game->player));
	return 1;
}

static int can_rotate(int bl, int x, int y, uint_least32_t *board)
{
	int row;
	int i = bl != TETR_I;
	if (i)
		i += !(bl & 0x444);
	if (x-i > 6)
		return 0;
	if (x + !(bl & 0x111) < 0)
		return 0;
	while (y < 0 || !(bl & 15)) {
		bl >>= 4;
		y++;
	}
	x *= 3;
	while (bl) {
		if (y > 19)
			return 0;
		row = board[y]>>x & 0xFFF;
		i = 1;
		while (row) {
			if (bl & i && row & 7)
				return 0;
			row >>= 3;
			i <<= 1;
		}
		bl >>= 4;
		y++;
	}
	return 1;
}

static void rotate_JL(struct player *plr, int bl)
{
	struct tetr *p = &plr->piece;
	int x = p->x;
	int y = p->y;
	if (can_rotate(bl, x, y, plr->board)) {
		drawpiece(plr, bl, x, y);
		clearblocks(plr, p->blocks ^ 0x20, x, y);
		p->blocks = bl;
	}
}

void rotate(struct player *plr, int clockwise)
{
	struct tetr *p = &plr->piece;
	uint_least32_t *board = plr->board;
	int y = p->y;
	int i = !(plr->rotationsys & ROT_LEFTHAND);

	switch (p->blocks) {
	case TETR_I:
		if (!can_rotate(TETR_I2, p->x+i, y, board))
			return;
		p->x += i;
		p->blocks = TETR_I2;
		drawpiece(plr, 0x1011, p->x+1, y);
		clearblocks(plr, i?11:13, p->x-i, y+2);
		break;
	case TETR_I2:
		if (!can_rotate(TETR_I, p->x-i, y, board))
			return;
		p->x -= i;
		p->blocks = TETR_I;
		drawpiece(plr, 15, p->x, y+2);
		clearblocks(plr, 0x1011, p->x+i+1, y);
		break;
	case TETR_J3:
		clockwise = !clockwise;
	case TETR_J:
		rotate_JL(plr, clockwise ? TETR_J2 : TETR_J4);
		break;
	case TETR_J4:
		clockwise = !clockwise;
	case TETR_J2:
		rotate_JL(plr, clockwise ? TETR_J3 : TETR_J);
		break;
	case TETR_L3:
		clockwise = !clockwise;
	case TETR_L:
		rotate_JL(plr, clockwise ? TETR_L2 : TETR_L4);
		break;
	case TETR_L4:
		clockwise = !clockwise;
	case TETR_L2:
		rotate_JL(plr, clockwise ? TETR_L3 : TETR_L);
		break;
	case TETR_S:
		if (!can_rotate(TETR_S2, p->x+i, y, board))
			return;
		p->blocks = TETR_S2;
		if (!i) {
			drawpiece(plr, 0x11, p->x, y);
			clearblocks(plr, 0x14, p->x, y+1);
		} else {
			p->x++;
			drawpiece(plr, 0x201, p->x, y);
			clearblocks(plr, 3, p->x-1, y+2);
		}
		break;
	case TETR_S2:
		if (!can_rotate(TETR_S, p->x-i, y, board))
			return;
		p->blocks = TETR_S;
		if (!i) {
			drawpiece(plr, 0x14, p->x, y+1);
			clearblocks(plr, 0x11, p->x, y);
		} else {
			p->x--;
			drawpiece(plr, 3, p->x, y+2);
			clearblocks(plr, 0x201, p->x+1, y);
		}
		break;
	case TETR_T:         
		i = clockwise ? TETR_T2 : TETR_T4;
		if (!can_rotate(i, p->x, y, board))
			return;
		p->blocks = i;
		drawpiece(plr, 1, p->x+1, y);
		clearblocks(plr, 1, p->x+(clockwise?2:0), y+1);
		break;
	case TETR_T2:
		i = clockwise ? TETR_T3 : TETR_T;
		if (!can_rotate(i, p->x, y, board))
			return;
		p->blocks = i;
		drawpiece(plr, 1, p->x+2, y+1);
		clearblocks(plr, 1, p->x+1, y+(clockwise?2:0));
		break;
	case TETR_T3:
		i = clockwise ? TETR_T4 : TETR_T2;
		if (!can_rotate(i, p->x, y, board))
			return;
		p->blocks = i;
		drawpiece(plr, 1, p->x+1, y+2);
		clearblocks(plr, 1, p->x+(clockwise?0:2), y+1);
		break;
	case TETR_T4:
		i = clockwise ? TETR_T : TETR_T3;
		if (!can_rotate(i, p->x, y, board))
			return;
		p->blocks = i;
		drawpiece(plr, 1, p->x, y+1);
		clearblocks(plr, 1, p->x+1, y+(clockwise?0:2));
		break;
	case TETR_Z:
		if (!can_rotate(TETR_Z2, p->x+i, y, board))
			return;
		p->blocks = TETR_Z2;
		if (!i) {
			drawpiece(plr, 0x102, p->x, y);
			clearblocks(plr, 3, p->x+1, y+2);
		} else {
			p->x++;
			drawpiece(plr, 0x11, p->x+1, y);
			clearblocks(plr, 0x41, p->x-1, y+1);
		}
		break;
	case TETR_Z2:
		if (!can_rotate(TETR_Z, p->x-i, y, board))
			return;
		p->blocks = TETR_Z;
		if (!i) {
			drawpiece(plr, 3, p->x+1, y+2);
			clearblocks(plr, 0x102, p->x, y);
		} else {
			p->x--;
			drawpiece(plr, 0x41, p->x, y+1);
			clearblocks(plr, 0x11, p->x+2, y);
		}
	}
	upd_dropmarker(plr, 0);
}

static int atspawnpos(struct tetr *p)
{
	const short bl[7] = {TETR_I,TETR_J,TETR_L,TETR_O,TETR_S,TETR_T,TETR_Z};
	struct tetr t;
	int i;
	if (p->x != 3 || p->y > 0)
		return 0;
	for (i = 0; i < 7; i++)
		if (p->blocks == bl[i]) {
			gettetrom(&t, i);
			return (p->y == t.y+2);
		}
	return 0;
}

static void movedown_n(struct player *plr, int drop, int n)
{
	struct tetr *p = &plr->piece;
	int bl = p->blocks;
	int y = p->y;
	int i, x;

	for (i = 0; i < n && !hitbtm(p, plr); i++) {
		p->y++; 
		if (drop)
			drop_score();
	}
	if (y == p->y)
		return;
	drawpiece(plr, p->blocks, p->x, p->y);
	i = 0;
	do {
		if (bl & 15) {
			x = 0;
			while (!(bl & 1<<x))
				x++;
			do if (y+i >= p->y && p->blocks & 1<<4*(i-p->y+y)+x)
				bl ^= 1 << x;
			while (bl & 1<<++x);
			clearblocks(plr, bl & 15, p->x, y+i);
		}
		i++;
	} while (y+i < 19 && (bl >>= 4));
}

int harddrop(struct player *plr, int safe)
{
	struct tetr *p = &plr->piece;
	if (safe && atspawnpos(p))
		return -1;
	movedown_n(plr, 1, 19);
	drawblocks(plr, p->blocks, p->x, p->y, DRAW_BLOCKS);
	lockpiece(plr);
	return 0;
}

int softdrop(int n, int safe)
{
	struct player *p = &player1;
	int t = gettm(0);
	int tm = -1;
	int y = p->piece.y;

	if (safe && atspawnpos(&p->piece))
		return -1;
	if (!movedown(p, 1))
		return 0;
	n--;
	while (n > 0) {
		y = p->piece.y;
		movedown_n(p, 1, 1+(n>1));
		processinput(tm-gettm(t)+t, DISCARD_DROPS);
		t = gettm(0);
		if (tm == -1) {
			tm = 2*p->falltime/20;
			if (tm > 10)
				tm = 10;
		}
		if (hitbtm(&p->piece, p))
			return 2;
		n -= p->piece.y-y;
	}
	return 1;
}

static int newclr(int old)
{
	int clr = randnum(7)+1;
	if (clr == old)
		return newclr(0);
	return clr;
}

static void gen_garbage_height(struct player *p, int h)
{
	int c, clr = randnum(7)+1;
	int x, i;
	if (p > game->player && p->height == player1.height) {
		memcpy(p->board, player1.board, 20*sizeof(uint_least32_t));
		return;
	}
	switch (h) {
	case 1:
	case 2:
		h = 2*h+1;
		break;
	default:
		h = 2*(h+1);
	}
	while (h) {
		i = 0;
		for (x = 0; x < 10; x++) {
			if (!randnum(2))
				continue;
			i++;
			if (i == 10)
				break;
			if (x && !(p->board[20-h]>>3*(x-1) & 7)) {
				c = p->board[19-h]>>3*x & 7;
				if (c && c != clr)
					clr = c;
				else
					clr = newclr(clr);
			}
			p->board[20-h] |= (uint_least32_t) clr << 3*x;
		}
		h--;
	}
}

void setupplayer(struct player *p)
{
	p->piece.blocks = 0;
	p->piece.y = 0;
	memset(p->board, 0, 20*sizeof(int_least32_t));
	if (p->height)
		gen_garbage_height(p, p->height);
	if  (!(game->mode & MODE_2PLAYER)) {
		p->score = 0;
		memset(tetr_stats, 0, 7);
	}
	p->level = -1;
	while (p->level < p->startlevel)
		levelup(p);
	p->lines = (game->mode & MODE_BTYPE) ? p->lineslimit : 0;
}

static int nextpiece(struct tetr *next)
{
	int i = randnum(7);
	player1.piece = *next;
	gettetrom(next, i);
	tetr_stats[i]++;
	drawnext(&player1, next);
	return movedown(&player1, 0) && movedown(&player1, 0);
}

int startgame_1p()
{
	struct tetr next;
	int i = randnum(7);
	int t;
	drawgamescreen_1p();
	gettetrom(&next, i);
	drawnext(&player1, &next);
	game->next = &next;
	if (!startgame_wait(SINGLE_PL))
		return 0;
	tetr_stats[i] = 1;
	upd_stat(&player1, 0);
	while (nextpiece(&next)) {
		spawn_discard_drops(0);
		t = gettm(0);
		show_dropmarker(&player1);
		while (1) {
			t = player1.falltime-gettm(t)+t;
			i = processinput(t, 0);
			if (!i)
				break;
			if (i == -2)
				return 0;
gravity:		t = gettm(0);
			if (i == 2 || movedown(&player1, 0))
				continue;
			i = lockdelay();
			if (i == -2)
				return 0;
			if (i)
				goto gravity;
			lockpiece(&player1);
			break;
		}
		if (player1.piece.y < 0)
			break;
		if (processinput(SPAWN_DELAY, DISCARD_MOVES) == -2)
			return 0;
	}
	game->state = GAME_OVER;
	return 1;
}

int startgame_wait(int n)
{
	int key;
	print_press_key();
	game->state = 0;
	key = getkeypress_block(n);
	n = processkey_ingame(key, DISCARD_MOVES | NO_PAUSE);
	if (n == -2)
		return 0;
	upd_screen(1);
	if (game->mode & MODE_2PLAYER)
		upd_screen(2);
	game->state = GAME_RUNNING;
	game->next = NULL;
	return 1;
}

int pausegame()
{
	int t = gettm(0);
	int key;
	game->state = GAME_PAUSED;
	clearboard_paused();
	hide_dropmarker(&player1);
	drawnext(&player1, NULL);
	do key = getkeypress_block(SINGLE_PL);
	while (gettm(t)-t < 500);
	if (processkey_ingame(key, NO_PAUSE | DISCARD_MOVES) == -2)
		return -2;
	upd_screen(1);
	show_dropmarker(&player1);
	game->state = GAME_RUNNING;
	return 2;
}

static int processinput(int tm, int flags)
{
	int key;
	int t = gettm(0);
	int d = 0;
	int ret;
	upd_screen(1);
	while (d < tm) {
		ret = -1;
		if (key = getkeypress(tm-d, IN_GAME | SINGLE_PL)) {
			ret = processkey_ingame(key, flags);
			if (!ret) {
				lockpiece(&player1);
				return 0;
			}
		}
		if (abs(ret) == 2 || ret > -1 && (flags & RETURN_ON_INPUT))
			return ret;
		d = gettm(t)-t;
	}
	if (!(flags & (DISCARD_MOVES | NO_PAUSE | RETURN_ON_INPUT))) {
		tm -= d;
		d = 0;
		while (tm <= -player1.falltime) {
			tm += player1.falltime;
			d++;
		}
		if (d) {
			movedown_n(&player1, 0, d);
			upd_screen(1);
		}
	}
	return 1;
}
