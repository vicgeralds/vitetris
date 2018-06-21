#include "menu.h"
#include "internal.h"
#include "../input/input.h"
#include "../game/tetris.h"
#include "../lang.h"
#include "../options.h"

static int gm_level(int k, int *pos)
{
	struct player *p = game->player+(player_==2);
	int i;
	if (k==MVLEFT)
		k = -1;
	else if (k==MVRIGHT)
		k = 1;
	else if (k >= '0' && k <= '9') {
		p->startlevel = 0;
		k -= '0';
	} else if (k)
		return 0;
	i = p->startlevel+k;
	if (i >= 0 && i < 10) {
		p->startlevel = i;
		printmenuitem_options("0 1 2 3 4 5 6 7 8 9", i);
	}
	return 1;
}

static int gm_height(int k, int *pos)
{
	struct player *p = game->player+(player_==2);
	int i;
	if (k==MVLEFT)
		k = -1;
	else if (k==MVRIGHT)
		k = 1;
	else if (k >= '0' && k <= '5') {
		p->height = 0;
		k -= '0';
	} else if (k)
		return 0;
	i = p->height+k;
	if (i >= 0 && i <= 5) {
		p->height = i;
		printmenuitem_options("0 1 2 3 4 5", i);
	}
	return 1;
}

static int gm_rotate(int k, int *pos)
{
	char s[25] = "clockwise anticlockw.";
	struct player *p = game->player+(player_==2);
	spellword(s+10);
	if (k==MVLEFT)
		p->rotationsys |= ROT_CLOCKWISE;
	else if (k==MVRIGHT)
		p->rotationsys &= ~ROT_CLOCKWISE;
	else if (k)
		return 0;
	printmenuitem_options(s, !(p->rotationsys & ROT_CLOCKWISE));
	return 1;
}

static int gm_orient(int k, int *pos)
{
	struct player *p = game->player+(player_==2);
	if (k==MVLEFT)
		p->rotationsys |= ROT_LEFTHAND;
	else if (k==MVRIGHT)
		p->rotationsys &= ~ROT_LEFTHAND;
	else if (k)
		return 0;
	printmenuitem_options("left right -handed", !(p->rotationsys &
						      ROT_LEFTHAND));
	return 1;
}

static int gm_softdrop(int k, int *pos)
{
	const char steps[5] = {1, 2, 3, 5, 20};
	int i;
	for (i = 0; i < 4; i++)
		if (steps[i] >= softdrop_speed)
			break;
	if (k==MVLEFT) {
		if (!i)
			return 1;
		i--;
	} else if (k==MVRIGHT) {
		if (i==4)
			return 1;
		i++;
	} else if (k)
		return 0;
	if (k)
		softdrop_speed = steps[i];
	printmenuitem_options("1 2 3 5 20", i);
	return 1;
}

int game_menu(int i, int x, int y)
{
	const char *menu[GAMEMENU_LENGTH] = {
		"Level",
		"Height",
		"-",
		"Input Setup",
		"Rotate",
		"Rotation Sys",
		"Softdrop Speed"
#ifndef TWOPLAYER
		,"-",
		"Options",
		"Highscores"
#endif
	};
	menuhandler handlers[GAMEMENU_LENGTH] = {
		gm_level, gm_height, NULL,
		inputsetup_menuitem, gm_rotate, gm_orient,
		gm_softdrop
#ifndef TWOPLAYER
		,NULL, rarrow_menuitem, rarrow_menuitem
#endif
	};
	struct player *p = &player1;
	const char *s = "";
	union val v;
#ifdef TWOPLAYER
	player_ = 0;
#endif
	if (socket_fd > -1)
		i = gamemenu_socket(menu, i, x, y, handlers);
	else if (TWOPLAYER_MODE) {
		if (game->mode & MODE_NETWORK)
			return 0;
		i = gamemenu_2p(menu, x, y, handlers);
	} else
		i = openmenu(menu, GAMEMENU_LENGTH, i, x, y, handlers);
	if (i==4)
		return 4;
	if (!i)
		return 0;
#ifndef TWOPLAYER
	if (i >= GAMEMENU_LENGTH-2)
		return i;
#endif
	if (TWOPLAYER_MODE && socket_fd == -1)
		s = "player1";
	while (1) {
		v.integ = p->startlevel;
		setoption(s, "level", v, 0);
		v.integ = p->height;
		setoption(s, "height", v, 0);
		v.integ = p->rotationsys;
		setoption(s, "rotate", v, 0);
		if (!TWOPLAYER_MODE) {
			v.integ = softdrop_speed;
			setoption("", "softdrop", v, 0);
			break;
		}
		if (p == &player2 || socket_fd > -1)
			break;
		p = &player2;
		s = "player2";
	}
	return 1;
}
