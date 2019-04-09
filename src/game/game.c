#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "game.h"
#include "tetris.h"
#include "tetris2p.h"
#include "../options.h"
#include "../input/input.h"
#include "../net/sock.h"
#include "../hiscore.h"
#include "../menu/menu.h"

extern int in_menu;

struct game game = {GAME_NULL};

int testgameopt(const char *key, int val, int pl)
{
	const char *keys[6] = {"mode", "level", "height",
			       "lines","rotate","softdrop"};
	const char max[6] = {127, 9, 5, 127, 7, 20};
	int i = 0;
	int n = 6;
	if (pl) { i++; n--; }
	for (; i < n; i++)
		if (!strcmp(key, keys[i]))
			return val >= 0 && val <= max[i] ? i+1 : -1;
	return 0;
}

static void initplayer(struct player *p, struct option *o, int pl)
{
	char gameopt[5] = {0, 0, 25, DEFAULT_ROTATION, 1};
	int v, i;
	if (game.mode & MODE_40L)
		gameopt[2] = 40;
	for (; o; o=o->next) {
		v = o->val.integ;
		i = testgameopt(opt_key(o), v, pl);
		i -= 2;
		if (i>=0)
			gameopt[i] = v;
#ifdef SOCKET
		else if (!strcmp(opt_key(o), "name")) {
			if (opt_isint(o))
				sprintf(my_name, "%d", v);
			else
				strncpy(my_name, opt_longstr(o), 16);
		}
#endif
	}
	p->startlevel  = gameopt[0];
	p->height      = gameopt[1];
	p->lineslimit  = gameopt[2];
	p->rotation    = gameopt[3];
	softdrop_speed = gameopt[4];
	p->next = NULL;
	p->score = 0;
	if (p->rotation & ROT_MODERN)
		p->rotation &= ~ROT_LEFTHAND;
}

void creategame()
{
	srand(time(NULL));
	game.state = GAME_CREATED;
	game.mode  = getopt_int("", "mode");
	if (game.mode & MODE_40L)
		game.mode &= ~MODE_B;
	if (socket_fd > -1) {
		game.mode |= MODE_2P | MODE_NET;
		initplayer(&player1, getoptions(""), 1);
		initplayer(&player2, NULL, 2);
	} else if (game.mode & MODE_2P) {
		initplayer(&player1, getoptions("player1"), 1);
		initplayer(&player2, getoptions("player2"), 2);
	} else {
		game.mode |= MODE_1P;
		game.mode &= MODE_1P | MODE_B | MODE_40L;
		initplayer(&player1, getoptions(""), 0);
	}
#ifdef SOCKET
	if (socket_fd > -1) {
		if (in_menu)
			request_playerlist();
		sock_initgame();
	} else
#endif
#ifdef JOYSTICK
	if (game.mode & MODE_2P)
		initplayerinput()
#endif
		;
}

int startgame()
{
	setupplayer(&player1);
	if (game.mode & MODE_2P) {
		setupplayer(&player2);
#ifdef SOCKET
		if (socket_fd < 0)
#endif
			game.mode &= ~MODE_NET;
		if (!startgame_2p())
			return 0;
		return 0;
	}
	readhiscores(NULL);
	return startgame_1p() && gameovermenu();
}
