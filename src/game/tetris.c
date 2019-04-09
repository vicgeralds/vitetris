#include <stdlib.h>
#include <string.h>
#include "tetris.h"
#include "../timer.h"
#include "../input/input.h"
#include "../draw.h"

static char sevenbag[8] = {0,0,0,0,0,0,0,7};
static int tm_40L;

static const short tetroms[4][7] = {
	{TETR_I,  TETR_J,  TETR_L,  TETR_O, TETR_S,  TETR_T,  TETR_Z },
	{TETR_I2, TETR_J2, TETR_L2, TETR_O, TETR_S2, TETR_T2, TETR_Z2},
	{TETR_I,  TETR_J3, TETR_L3, TETR_O, TETR_S,  TETR_T3, TETR_Z },
	{TETR_I2, TETR_J4, TETR_L4, TETR_O, TETR_S2, TETR_T4, TETR_Z2}
};

#define RETURN_ON_INPUT 4

/* process single-player input */
static int processinput(int tm, int flags);

int randnum(int n)
{
	return (int) (n*(rand()/(RAND_MAX+1.0)));
}

/* 7-bag random generator */
static void sevenbag_gen()
{
	int nums[7] = {0,1,2,3,4,5,6};
	div_t q;
	int d = 720;
	int i, j;
	q.rem = randnum(5040);
	for (i=0; i<6; i++) {
		q = div(q.rem, d);
		sevenbag[i] = nums[q.quot];
		for (j=q.quot; j < 6-i; j++)
			nums[j] = nums[j+1];
		d /= 6-i;
	}
	sevenbag[6] = nums[0];
	sevenbag[7] = 0;
}

static int sevenbag_next()
{
	int i;
	if (sevenbag[7] >= 7)
		sevenbag_gen();
	i = sevenbag[7];
	sevenbag[7]++;
	return sevenbag[i];
}

int rand_tetrom_next()
{
	if (game.mode & MODE_40L)
		return sevenbag_next();
	return randnum(7);
}

void gettetrom(struct tetr *t, int i, int rot)
{
	int state = 0;
	if (rot & ROT_MODERN)
		state = 2;
	t->blocks = tetroms[state][i];
	t->x = 3;
	t->y = -3;
	t->color = tetrom_colors[i];
	t->state = state;
	if (i==0) t->y = -4;
	if (i==3) t->y = -2;
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
	if (!(game.mode & (MODE_2P | MODE_40L))) {
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
						/* 10:00.00 */
	if (game.mode == MODE_1P_40L && p->score >= 60000)
		p->piece.y = -1;
	if (game.mode & MODE_LINECLEAR) {
		p->lines -= n;
		if (p->lines <= 0) {
			p->lines = 0;
			p->piece.y = -1;
		}
	} else {
		p->lines += n;
		if (p->level == p->startlevel) {
			if (p->lines >= 10*p->startlevel+10 || p->lines >= 100)
				levelup(p);
		} else if (p->lines/10 != (p->lines-n)/10)
			levelup(p);
	}
}

static void clearlines(struct player *p, int n)
{
	uint_least32_t *board = p->board;
	unsigned char lines[4];
	int i;
	upd_score_level(p, n);
	memcpy(lines, game.clearedlines, 4);
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
	const char *lines = game.clearedlines;
	int x, i;
	int t = gettm(0);
	int tm;
	for (x = 4; x >= 0; x--) {
		for (i = 0; i < n; i++) {
			clearblocks(&player1, 1, x, lines[i]-1);
			clearblocks(&player1, 1, 9-x, lines[i]-1);
		}
		tm = CLEAR_DELAY/5;
		tm = (5-x)*tm-gettm(t)+t;
		processinput(tm, DISCARD_MOVES | NO_PAUSE);
	}
	redrawboard(&player1, lines[n-1]-1);
}

int test_lockdelay_move(struct player *plr)
{
	struct tetr      *p  = &plr->piece;
	struct lockdelay *ld = &plr->lockdelay;
	int i;
	for (i=0; i < ld->num_moves; i++) {
		if (p->blocks == ld->moves[i].blocks &&
		    p->x      == ld->moves[i].x  &&
		    p->y      == ld->moves[i].y)
			return 0;
	}
	if (i >= LOCK_DELAY_MOVES)
		return 0;
	ld->num_moves++;
	ld->moves[i].blocks = p->blocks;
	ld->moves[i].x  = p->x;
	ld->moves[i].y  = p->y;
	return 1;
}

static int lockdelay()
{
	int t = gettm(0);
	int d = 0;

	player1.lockdelay.num_moves = 0;

	while (d < LOCK_DELAY) {
		switch (processinput(LOCK_DELAY-d, RETURN_ON_INPUT)) {
		case -2:
			return -2;
		case 0:
			return 0;
		}
		if (!hitbtm(&player1.piece, &player1))
			return 1;
		if (test_lockdelay_move(&player1)) {
			/* reset lock delay */
			d = gettm(t)-t-LOCK_DELAY;
			if (d < 0)
				d = 0;
			else if (d > LOCK_DELAY-10)
				d = LOCK_DELAY-10;
			t = gettm(0)-d;
		} else
			d = gettm(t)-t;
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
	plr->initrot = 0;
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
	memset(game.clearedlines, 0, 4);
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
			game.clearedlines[n++] = y+1;
		y++;
	} while (p->blocks >>= 4);
	x = plr->level;
	if (n)
		clearlines(plr, n);
	upd_stat(plr, x != plr->level);
	if (n && !(game.mode & MODE_2P))
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
	if (!(game.mode & (MODE_2P | MODE_40L)))
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
out:	upd_screen(1+(plr > game.player));
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
	while (x < 0 && !(bl & 0x111)) {
		bl >>= 1;
		x++;
	}
	if (x < 0)
		return 0;
	while (y < 0 || !(bl & 15)) {
		bl >>= 4;
		if (!bl)
			return 1;
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

static int
test_wallkicks(struct player *plr, int bl, int *xp, int *yp, int state)
{
	struct tetr *p = &plr->piece;
	int x = *xp;
	int y = *yp;
	int floorkick = 0;

	if (bl == TETR_I2) {
		if (plr->floorkick > 1)
			return 0;
		floorkick = 1 + (p->state==0);
		y--;
		if (can_rotate(bl, x, y, plr->board)) {
			*yp = y;
			plr->floorkick = floorkick;
			return 1;
		}
		if (p->state==0)
			return 0;
		y--;
		floorkick = 2;
	} else {
		if (p->state==3 || state==1)
			x++;
		if (p->state==1 || state==3)
			x--;
		*xp = x;
		if (can_rotate(bl, x, y, plr->board))
			return 1;
		if (state==2)
			y++;
		else if (p->state==2 && !plr->floorkick) {
			y--;
			floorkick = 1;
		} else
			return 0;
	}
	if (can_rotate(bl, x, y, plr->board)) {
		*yp = y;
		if (floorkick)
			plr->floorkick = floorkick;
		return 1;
	}
	return 0;
}

static void redrawpiece(struct player *plr, int bl, int oldbl, int x, int y)
{
	int overlap = bl & oldbl;
	drawpiece(plr, bl ^ overlap, x, y);
	clearblocks(plr, oldbl ^ overlap, x, y);
}

static void do_rotate(struct player *plr, int bl, int x, int y, int state)
{
	struct tetr *p = &plr->piece;
	int oldbl = p->blocks;
	int oldx  = p->x;
	int oldy  = p->y;

	p->blocks = bl;
	p->x      = x;
	p->y      = y;
	p->state  = state;

	if (x < oldx)
		oldbl <<= 1;
	else if (x > oldx) {
		bl <<= 1;
		x = oldx;
	}
	if (y > oldy) {
		if (oldbl & 15) {
			bl <<= 4;
			y = oldy;
		} else
			oldbl >>= 4;
	} else if (y < oldy) {
		if (bl & 15)
			oldbl <<= 4;
		else {
			bl >>= 4;
			y = oldy;
		}
	}
	redrawpiece(plr, bl, oldbl, x, y);
}

void rotate(struct player *plr, int clockwise)
{
	struct tetr *p = &plr->piece;
	int bl    = p->blocks;
	int x     = p->x;
	int y     = p->y;
	int state = p->state;
	int i;

	if (bl == TETR_O)
		return;
	for (i=0; bl != tetroms[state][i]; i++)
		if (i >= 6)
			return;
	if (clockwise)
		state++;
	else
		state += -1+4;
	state %= 4;
	bl = tetroms[state][i];
	if (!(plr->rotation & ROT_LEFTHAND)) {
		switch (i) {
		case 0:  /* I */
		case 4:  /* S */
		case 6:  /* Z */
			if (!(plr->rotation & ROT_MODERN)) {
				state %= 2;
				if (state==1)
					state = 3;
			}
			if (state==2)
				y--;
			else if (p->state==2)
				y++;
			if (state==3)
				x++;
			else if (p->state==3)
				x--;
		}
	}
	if (!can_rotate(bl, x, y, plr->board)) {
		if (!(plr->rotation & ROT_MODERN))
			return;
		if (!test_wallkicks(plr, bl, &x, &y, state))
			return;
	}
	do_rotate(plr, bl, x, y, state);
	upd_dropmarker(plr, 0);
}

static int atspawnpos(struct tetr *p, int rot)
{
	const short *bl = tetroms[0];
	struct tetr t;
	int i;
	if (p->x != 3 || p->y > 0)
		return 0;
	if (rot & ROT_MODERN)
		bl = tetroms[2];
	for (i = 0; i < 7; i++)
		if (p->blocks == bl[i]) {
			gettetrom(&t, i, rot);
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
	if (safe && atspawnpos(p, plr->rotation))
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

	if (safe && atspawnpos(&p->piece, p->rotation))
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

void initialrotate(struct player *plr, int clockwise)
{
	if (clockwise)
		plr->initrot++;
	else
		plr->initrot--;
}

int spawnpiece(struct player *plr)
{
	int cw = plr->initrot > 0;
	while (plr->initrot) {
		rotate(plr, cw);
		if (cw)
			plr->initrot--;
		else
			plr->initrot++;
	}
	plr->floorkick = 0;
	if (movedown(plr, 0) && movedown(plr, 0)) {
		show_dropmarker(plr);
		upd_screen(0);
		return 1;
	}
	return 0;
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
	if (p > game.player && p->height == player1.height) {
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
	if  (!(game.mode & MODE_2P)) {
		p->score = 0;
		memset(tetr_stats, 0, 7);
	}
	p->level = -1;
	while (p->level < p->startlevel)
		levelup(p);
	p->initrot = 0;
	p->lines = 0;
	if (game.mode & MODE_LINECLEAR)
		p->lines = p->lineslimit;
}

static int nextpiece(struct tetr *next)
{
	int i = rand_tetrom_next();
	player1.piece = *next;
	gettetrom(next, i, player1.rotation);
	tetr_stats[i]++;
	drawnext(&player1, next);
	return spawnpiece(&player1);
}

int startgame_1p()
{
	struct tetr next;
	int i = rand_tetrom_next();
	int t;
	drawgamescreen_1p();
	gettetrom(&next, i, player1.rotation);
	drawnext(&player1, &next);
	player1.next = &next;
	if (!startgame_wait(SINGLE_PL))
		return 0;
	tetr_stats[i] = 1;
	upd_stat(&player1, 0);
	tm_40L = gettm(0);
	while (nextpiece(&next)) {
		spawn_discard_drops(0);
		t = gettm(0);
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
	game.state = GAME_OVER;
	return 1;
}

int startgame_wait(int n)
{
	int key;
	print_press_key();
	game.state = GAME_CREATED;
	key = getkeypress_block(n);
	n = processkey_ingame(key, DISCARD_MOVES | NO_PAUSE);
	if (n == -2)
		return 0;
	upd_screen(1);
	if (game.mode & MODE_2P) {
		upd_screen(2);
		player2.next = NULL;
	}
	player1.next = NULL;
	game.state = GAME_RUNNING;
	return 1;
}

int pausegame()
{
	int t = gettm(0);
	int key;
	game.state = GAME_PAUSED;
	clearboard_paused();
	hide_dropmarker(&player1);
	drawnext(&player1, NULL);
	if (game.mode == MODE_1P_40L)
		player1.score++;
	do key = getkeypress_block(SINGLE_PL);
	while (gettm(t)-t < 500);
	if (processkey_ingame(key, NO_PAUSE | DISCARD_MOVES) == -2)
		return -2;
	tm_40L = gettm(0);
	upd_screen(1);
	show_dropmarker(&player1);
	game.state = GAME_RUNNING;
	return 2;
}

static void update_40L_time()
{
	int d;
	if (game.mode == MODE_1P_40L) {
		d = gettm(tm_40L)-tm_40L;
		while (d >= 10) {
			player1.score++;
			d -= 10;
			tm_40L += 10;
		}
		if (tm_40L > 1000*TIMER_WRAPAROUND_SECS)
			tm_40L -= 1000*TIMER_WRAPAROUND_SECS;
		upd_stat(&player1, 0);
	}
}

static int processinput(int tm, int flags)
{
	int key;
	int t = gettm(0);
	int d = 0;
	int ret;
	update_40L_time();
	upd_screen(1);
	while (d < tm) {
		ret = -1;
		if (key = getkeypress(tm-d, IN_GAME | SINGLE_PL)) {
			update_40L_time();
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
	update_40L_time();
	if (!(flags & (DISCARD_MOVES | NO_PAUSE | RETURN_ON_INPUT))) {
		tm -= d;
		d = 0;
		/* falltime must be > 0 */
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
