#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "game.h"
#include "tetris.h"
#include "tetris2p.h"
#include "../options.h"
#include "../input/input.h"
#include "../netw/sock.h"
#include "../hiscore.h"
#include "../menu/menu.h"

extern int in_menu;

int testgameopt(const char *key, int val, int pl)
{
	const char *keys[6] = {"mode", "level", "height",
			       "lines","rotate","softdrop"};
	const char max[6] = {127, 9, 5, 127, 3, 20};
	int i = 0;
	int n = 6;
	if (pl) { i++; n--; }
	for (; i < n; i++)
		if (!strcmp(key, keys[i]))
			return val >= 0 && val <= max[i] ? i+1 : -1;
	return 0;
}

static void initplayer(struct player *p)
{
	p->startlevel = 0;
	p->height = 0;
	p->lineslimit = 25;
	p->rotationsys = ROT_CLOCKWISE | ROT_LEFTHAND;
	p->score = 0;
}

void initgame()
{
	struct player *p = game->player;
	int pl = 0;
	struct option *o = getoptions("");
	int v;
	int i;
	srand(time(NULL));
	game->mode = getopt_int("", "mode");
#ifndef TWOPLAYER
	game->mode &= ~(MODE_2PLAYER | MODE_NETWORK);
#else
	if (socket_fd > -1)
		game->mode |= MODE_2PLAYER | MODE_NETWORK;
#endif
	game->state = 0;
	game->next = NULL;
	initplayer(p);
	if (!TWOPLAYER_MODE)
		softdrop_speed = 1;
	else {
		initplayer(p+1);
		pl = 1;
		if (socket_fd == -1)
			o = getoptions("player1");
	}
	while (1) {
		if (!o) {
			if (!TWOPLAYER_MODE || pl==2 || socket_fd > -1)
				break;
			p++;
			pl = 2;
			o = getoptions("player2");
			continue;
		}
		v = o->val.integ;
		i = testgameopt(opt_key(o), v, pl);
		if (i>=2 && i<=5)
			(&p->startlevel)[i-2] = v;
		else if (i==6)
			softdrop_speed = v;
#ifdef SOCKET
		else if (!strcmp(opt_key(o), "name")) {
			if (opt_isint(o))
				sprintf(my_name, "%d", v);
			else
				strncpy(my_name, opt_longstr(o), 16);
		}
#endif
		o = o->next;
	}
#ifdef SOCKET
	if (socket_fd > -1) {
# ifndef NO_MENU
		if (in_menu)
			request_playerlist();
# endif
		sock_initgame();
	} else
#endif
#if TWOPLAYER && JOYSTICK
	if (TWOPLAYER_MODE)
		initplayerinput()
#endif
		;
}

int startgame()
{
	setupplayer(&player1);
	if (TWOPLAYER_MODE) {
		setupplayer(&player2);
#ifdef SOCKET
		if (socket_fd < 0)
#endif
			game->mode &= ~MODE_NETWORK;
		if (!startgame_2p())
			return 0;
#ifdef NO_MENU
		return 1;
#else
		return 0;
#endif
	}
	if (!hiscores[0].score)
		readhiscores(NULL);
	return startgame_1p() && gameovermenu();
}
