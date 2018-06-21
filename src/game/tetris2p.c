#include <stdlib.h>
#include <string.h>
#undef TWOPLAYER
#define TWOPLAYER 1
#include "tetris.h"
#include "tetris2p.h"
#include "game.h"	/* wonlost stats */
#include "../timer.h"
#include "../input/input.h"
#include "../draw/draw.h"
#ifndef SOCKET
#define SOCKET_EMPTY_DEFS 1
#endif
#include "../netw/sock.h"

struct tetris2p tetris2p[2];
struct player *winner = NULL;

char *tetrom_seq = NULL;
static int tetrom_seq_len = -1;
static int tetrom_seq_pos[2];
#ifdef SOCKET
static int pl2_has_next;
#endif

static void free_tetrom_seq()
{
	if (tetrom_seq) {
		free(tetrom_seq);
		tetrom_seq = NULL;
		tetrom_seq_len = 0;
		tetrom_seq_pos[0] = 0;
		tetrom_seq_pos[1] = 0;
	}
}

static void gen_tetrom_seq(int i, int n)
{
	n += i;
	if (n > tetrom_seq_len) {
		tetrom_seq = !tetrom_seq ? malloc(n)
					 : realloc(tetrom_seq, n);
		if (tetrom_seq_len == -1)
			atexit(free_tetrom_seq);
		tetrom_seq_len = n;
	}
	for (; i < n; i++)
		tetrom_seq[i] = randnum(7);
}

static int
draw_lineclear_step(struct player *plr, struct tetris2p *tet, int first)
{
	char *lines = tet->clearedlines;
	int n, x;
	n = strlen(lines);
	if (first)
		tet->x1 = 6;
	x = --tet->x1;
	if (x) {
		while (n) {
			n--;
			clearblocks(plr, 1, x-1, lines[n]-1);
			clearblocks(plr, 1, 10-x, lines[n]-1);
		}
		upd_screen(1+(tet>tetris2p));
		tet->delay = CLEAR_DELAY/5;
	} else {
		redrawboard(plr, lines[n-1]-1);
		memset(lines, 0, 4);
	}
	return x;
}

static void sendgarbage(struct tetris2p *tet)
{
	int n = strlen(tet->clearedlines);
	int i;
	if (n < 4)
		n--;
	if (n > 0) {
		i = tet==tetris2p;
		tetris2p[i].garbage[0] += n;
		if (tetris2p[i].garbage[0] > 12)
			tetris2p[i].garbage[0] = 12;
		upd_garbagemeter(game->player+i, tetris2p[i].garbage[0]);
	}
}

static void getgarbage_row(struct player *plr, char *garbg)
{
	int i, clr;
	uint_least32_t bl;
	if (!garbg[1] || garbg[2] == 8) {
		do i = randnum(10)+1;
		while (i == garbg[1]);
		garbg[1] = i;
		garbg[2] = 0;
	}
	garbg[2]++;
	bl = plr->board[18];
	if (!bl)
		clr = 1;
	else {
		while (!(bl & 7))
			bl >>= 3;
		clr = (bl & 7);
	}
	for (i = 0; i < 10; i++)
		if (i+1 != garbg[1])
			plr->board[19] |= clr << 3*i;
}

static void getgarbage(struct player *plr, char *garbg)
{
	int i;
	if (!garbg[0])
		return;
	if (NETPLAY && plr == &player1)
		sock_sendgarbage_num(garbg[0]+12);
	while (garbg[0] && !plr->board[0]) {
		for (i = 0; i < 19; i++)
			plr->board[i] = plr->board[i+1];
		plr->board[19] = 0;
		if (!NETPLAY || plr == &player1)
			getgarbage_row(plr, garbg);
		redrawboard(plr, 19);
		garbg[0]--;
	}
	hide_dropmarker(plr);
	upd_garbagemeter(plr, garbg[0]);
	if (garbg[0])
		plr->piece.y = -1;
}

static void spawndelay(struct player *plr, struct tetris2p *tet)
{
	tet->falltm = plr->falltime;
	tet->delay  = SPAWN_DELAY;
	if (!NETPLAY || tet==tetris2p)
		getgarbage(plr, tet->garbage);
#ifdef SOCKET
	else if (sock_flags & PL2_GARBAGE)
		getgarbage_player2();
#endif
}

#ifdef SOCKET
static void fix_delayed_garbage(int n)
{
	struct tetr *p = &player2.piece;
	p->y -= n;
	while (n && !hitbtm(p, &player2)) {
		p->y++;
		n--;
	}
	if (n) {
		redrawboard(&player2, 19);
		n = p->y;
		if (p->blocks == TETR_I)
			n += 2;
		else if (!(p->blocks & 7))
			n++;
		if (n < 0)	/* game over */
			tetris2p[1].delay = 1;
	}
}

void getgarbage_player2()
{
	char *garbg = tetris2p[1].garbage;
	int n = garbg[1];
	sock_sendgarbage_num(n);
	garbg[0] -= n;
	getgarbage(&player2, garbg+1);
	if (!garbg[1]) {
		if (garbg[0] > 0)
			upd_garbagemeter(&player2, garbg[0]);
		if (!tetris2p[1].delay)
			fix_delayed_garbage(n);
	}
	sock_flags &= ~PL2_GARBAGE;
}

static void sendnext(struct player *p)
{
	int i;
	if (is_server) {
		if (i = isplayer2(p)) {
			if (pl2_has_next)
				return;
			pl2_has_next = 1;
		}
		i = tetrom_seq_pos[i]+1;
		if (i >= tetrom_seq_len)
			gen_tetrom_seq(i, 1);
		sock_sendnext(p, tetrom_seq[i]);
	}
}
#else
#define sendnext(p)
#endif

static void lockdelay(struct player *plr, struct tetris2p *tet)
{
	struct tetr *p = &plr->piece;
	tet->falltm = 0;
	tet->delay = LOCK_DELAY;
	tet->x1 = p->x;
	tet->x2 = p->x;
	tet->b1 = p->blocks;
	tet->b2 = p->blocks;
	if (NETPLAY)
		sendnext(plr);
}

static int upd_lockdelay_state(struct player *plr, struct tetris2p *tet)
{
	struct tetr *p = &plr->piece;
	if (!hitbtm(p, plr)) {
		tet->delay = 0;
		tet->falltm = plr->falltime;
		tet->b1 = 0;
		movedown(plr, 0);
		return 1;
	}
	if (NETPLAY && tet>tetris2p) {
		tet->delay = LOCK_DELAY;
		return 1;
	}
	if (tet->delay <= 0)
		return 0;
	if (p->x != tet->x2) {
		if (p->x == tet->x1)
			return 0;
		tet->delay = LOCK_DELAY;
		tet->x1 = tet->x2;
		tet->x2 = p->x;
	} else if (p->blocks != tet->b2) {
		if (p->blocks == tet->b1)
			return 0;
		tet->delay = LOCK_DELAY;
		tet->b2 = p->blocks;
	}
	return 1;
}

static void lockpiece_2p(struct player *plr, struct tetris2p *tet)
{
	if (NETPLAY) {
		if (tet>tetris2p && plr->piece.blocks) {
			lockdelay(plr, tet);
			return;
		}
		if (!tet->b1)
			sendnext(plr);
		if (plr->piece.blocks) {
			sock_sendpiece(plr);
			sock_sendbyte(HARDDROP);
		}
	}
	lockpiece(plr);
	if (!clearedlines[0]) {
		upd_screen(1+(tet>tetris2p));
		spawndelay(plr, tet);
	} else {
		tet->falltm = 0;
		memcpy(tet->clearedlines, clearedlines, 4);
		draw_lineclear_step(plr, tet, 1);
		sendgarbage(tet);
	}
	tet->b1 = 0;
}

static void show_next(struct player *p, int i)
{
	struct tetr next;
	gettetrom(&next, i);
	drawnext(p, &next);
}

int nextpiece_2p(struct player *plr)
{
	struct tetris2p *tet = tetris2p;
	int *i = tetrom_seq_pos;
	int *j = tetrom_seq_pos;
	char *p;

	if (plr->piece.y < 0)
		return 0;
	if (plr == &player1) {
		j++;
		spawn_discard_drops(1);
	} else {
		i++;
		tet = tetris2p+1;
		spawn_discard_drops(2);
#ifdef SOCKET
		pl2_has_next = 0;
#endif
	}
	if (*i+1 >= tetrom_seq_len) {
		memmove(tetrom_seq, tetrom_seq+*j, *i-*j+1);
		*i -= *j;
		*j = 0;
		gen_tetrom_seq(*i+1, 8);
	}
	p = tetrom_seq+*i;
	if (!NETPLAY || is_server)
		*i += 1;
#ifdef SOCKET
	else if (*p==0x7F) {
		sock_flags &= ~PL2_IN_GAME;
		return 0;
	}
#endif
	gettetrom(&plr->piece, *p);
#ifdef SOCKET
	if (p[1]==0x7F)
		drawnext(plr, NULL);
	else
#endif
		show_next(plr, p[1]);
	if (NETPLAY && !is_server) {
		*p = p[1];
		p[1] = 0x7F;
	}
	tet->falltm = plr->falltime;
	tet->delay = 0;
	if (movedown(plr, 0) && movedown(plr, 0)) {
		show_dropmarker(plr);
		upd_screen(0);
		return 1;
	}
	return 0;
}

static int getwaittm()
{
	int tm = (tetris2p[0].delay) ? tetris2p[0].delay : tetris2p[0].falltm;
	if (tetris2p[1].delay) {
		if (tm > tetris2p[1].delay)
			tm = tetris2p[1].delay;
	} else if (tm > tetris2p[1].falltm)
		tm = tetris2p[1].falltm;
#ifdef SOCKET
	if (tm > 1000)
		tm = 1000;
#endif
	return tm;
}

static int processkey(int *pl2)
{
	struct tetris2p *tet = tetris2p;
	int key = getkeypress(getwaittm(), IN_GAME);
	int flags;
	*pl2 = 0;
	if (!key)
		return -1;
	if (key & PLAYER_2) {
		*pl2 = 1;
		flags = PLAYER_2;
		tet = tetris2p+1;
	} else if (NETPLAY && sock_flags & SAME_HEIGHT && !player1.height)
		flags = DISCARD_MOVES;
	else
		flags = 0;
	if (tet->delay && (tet->falltm || *tet->clearedlines))
		flags |= DISCARD_MOVES;
	return processkey_ingame(key, flags);
}

static int upd_player(struct player *plr, struct tetris2p *tet, int tm)
{
	if (tet->delay) {
		tet->delay -= tm;
		if (tet->falltm)
			return tet->delay > 0;
		if (tet->b1) {
			if (!upd_lockdelay_state(plr, tet))
				lockpiece_2p(plr, tet);
		} else if (tet->delay <= 0) {
			if (!draw_lineclear_step(plr, tet, 0))
				spawndelay(plr, tet);
		}
	} else {
		tet->falltm -= tm;
		if (tet->falltm <= 0) {
			if (movedown(plr, 0))
				tet->falltm = plr->falltime;
			else
				lockdelay(plr, tet);
		}
	}
	return 1;
}

static int startgame_wait_2p()
{
	struct tetr next = {0};
	gettetrom(&next, *tetrom_seq);
	game->next = &next;
	return startgame_wait(0);
}

static int wait_next()
{
#ifdef SOCKET
	int t = gettm(0);
	while (*tetrom_seq==0x7F) {
		sock_getkeypress(0);
		if (sock_flags & CONN_BROKEN || !(sock_flags & PL2_IN_GAME) ||
		    gettm(t)-t >= 1000)
			return 0;
	}
#endif
	return 1;
}

static int initround()
{
	memset(tetris2p, 0, 2*sizeof(struct tetris2p));
	winner = NULL;
	if (!NETPLAY || is_server) {
		gen_tetrom_seq(0, 8);
		show_next(&player1, *tetrom_seq);
		show_next(&player2, *tetrom_seq);
	} else {
		tetrom_seq = malloc(4);
		memset(tetrom_seq, 0x7F, 4);
		tetrom_seq_len = 4;
		tetrom_seq_pos[1] = 2;
	}
	if (NETPLAY)
		sock_initround();
	if (!startgame_wait_2p())
		return 0;
	if (NETPLAY) {
		if (*tetrom_seq==0x7F && !wait_next())
			return 0;
		sock_sendbyte('.');
	}
	return 1;
}

static int gameover_timeout(int *gameover)
{
	if (!NETPLAY || is_server)
		return 1;
#ifdef SOCKET
	if (*tetrom_seq==0x7F && !(sock_flags & PL2_IN_GAME)) {
		*gameover = 2;
		sock_flags |= PL2_IN_GAME;
		return 1;
	}
	if (*gameover == 1 && tetris2p[0].delay < 31768 && tetris2p[1].b1)
		return 1;
	if (*gameover == 3 && tetris2p[1].delay < 31768) {
		*gameover = 1 + (tetris2p[0].delay >= tetris2p[1].delay);
		return 1;
	}
	return 0;
#endif
}

static void sync_spawn_delay()
{
	if (NETPLAY && tetrom_seq_pos[0] == tetrom_seq_pos[1] &&
	    tetris2p[0].delay > 100 && tetris2p[1].delay > 100 &&
	    tetris2p[0].falltm && tetris2p[1].falltm == player1.falltime)
	{
		tetris2p[0].delay = tetris2p[1].delay;
	}
}

static void set_gameover(int *gameover, int i)
{
	*gameover |= 1+i;
	if (NETPLAY && !is_server) {
		tetris2p[i].falltm = -1;
		tetris2p[i].delay = 0x7FFF;
	}
}

static void updateplayer(int *gameover, int i, int t)
{
	struct player *plr = game->player+i;
	if (!upd_player(plr, tetris2p+i, gettm(t)-t) && !nextpiece_2p(plr))
		set_gameover(gameover, i);
}

static struct player *play_round()
{
	int gameover = 0;
	int t, i, ret;
	if (!initround())
		return NULL;
	nextpiece_2p(&player1);
	nextpiece_2p(&player2);
	while (!gameover || !gameover_timeout(&gameover)) {
		if (NETPLAY && !(sock_flags & PL2_IN_GAME)) {
			sock_sendbyte(ESC);
			return &player1;
		}
		t = gettm(0);
		sync_spawn_delay();
		ret = processkey(&i);
		if (ret == -2)
			return NULL;
		if (NETPLAY && i) {
			if (!tetris2p[1].delay && tetris2p[1].garbage[1]) {
				tetris2p[1].delay = 0x7FFF;
				set_gameover(&gameover, 1);
			}
			if (ret==1)
				sendnext(&player2);
		}
		if (!ret)
			lockpiece_2p(game->player+i, tetris2p+i);
		if (ret)
			updateplayer(&gameover, i, t);
		updateplayer(&gameover, !i, t);
		if (winner)
			return winner;
	}
	i = !(gameover & 1);
	if (!(game->mode & MODE_BTYPE) || game->player[i].lines)
		i = !i;
	if (gameover==3 && (!(game->mode & MODE_BTYPE) || !player1.lines ==
							  !player2.lines))
		i = 2;
	winner = game->player+i;
	if (NETPLAY)
		sock_sendwinner();
	return winner;
}

static int wait_gameover()
{
	int tm = 2000;
	int t;
	game->state = GAME_OVER;
	do {
		t = gettm(0);
		switch (getkeypress(tm, IN_GAME) & 0x7F) {
		case ESC:	return 0;
		case STARTBTN:	return 1;
		}
		tm -= gettm(t)-t;
	} while (tm > 0);
	return processkey_ingame(getkeypress_block(0), DISCARD_MOVES) != -2;
}

int startgame_2p()
{
	struct player *plr;
	player1.score = 0;
	player2.score = 0;
	drawgamescreen_2p();
	if (NETPLAY) {
		if (!sock_wait_pl2ingame())
			return 0;
		plr = &player2;
		if (plr->level != plr->startlevel) {
			setupplayer(plr);
			upd_stat(plr, 0);
		}
	}
	while (plr = play_round()) {
		free_tetrom_seq();
		if (plr == game->player+2)	/* draw */
			plr = NULL;
		else {
			plr->score++;
			upd_stat(plr, 0);
		}
		show_winner(plr);
		if (plr && plr->score == 3) {
#ifdef SOCKET
			if (NETPLAY && opponent_name[0])
				upd_wonlost_stats(my_name, opponent_name,
						  player1.score == 3);
#endif
			;
		} else
			plr = NULL;
		if (!wait_gameover() || plr)
			break;
		plr = &player1;
		if (NETPLAY && (SAME_HEIGHT|PL2_IN_GAME) ==
		    (sock_flags & (IS_SERVER|SAME_HEIGHT|PL2_IN_GAME)))
			plr->height = 0;
		while (1) {
			setupplayer(plr);
			redrawboard(plr, 19);
			upd_stat(plr, 0);
			upd_garbagemeter(plr, 0);
			hide_dropmarker(plr);
			if (isplayer2(plr))
				break;
			plr++;
		}
		if (NETPLAY) {
			if (!(sock_flags & PL2_IN_GAME)) {
				game->state = 0;
				return startgame_2p();
			}
			sock_sendbyte('s');
		}
	}
	free_tetrom_seq();
	game->state = 0;
#ifdef SOCKET
	if (NETPLAY) {
		sock_flags &= ~PL2_IN_GAME;
		sock_sendbyte(ESC);
	}
#endif
	return plr != NULL;
}
