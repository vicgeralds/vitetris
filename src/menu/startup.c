#include "menu.h"
#include "menuext.h"
#include "../options.h"
#include "../game/tetris.h"	/* MODE_BTYPE */

static int gm_mode(int k, int *pos)
{
	const char *items[2] = {"A-type", "B-type"};
	union val v;
	int i;
	int ret;
	v.integ = getopt_int("", "mode");
	i = 1-!(v.integ & MODE_BTYPE);
	ret = selectitem(items, 2, &i, k);
	if (ret && k) {
		if (i)
			v.integ |= MODE_BTYPE;
		else
			v.integ &= ~MODE_BTYPE;
		setoption("", "mode", v, 0);
	}
	return ret;
}

int startup_menu(int i, int x, int y)
{
	union val v;
	const char *menu[7] = {
		"1-Player Game",
		"2-Player Game",
#ifdef INET
		"Netplay",
#endif
		"Mode",
		"-------------",
		"Options",
		"Highscores"
	};
	menuhandler handlers[7] = {0, 0,
#ifdef INET
		0,
#endif
		gm_mode};
	i = openmenu(menu,
#ifdef INET
				1 +
#endif
				6, i, x, y, handlers);
	if (i && i <= 3) {
#ifdef INET
		if (i==3)
			v.integ = MODE_2PLAYER | MODE_NETWORK;
		else
#endif
		v.integ = i;
		v.integ |= getopt_int("", "mode") & MODE_BTYPE;
		setoption("", "mode", v, 0);
	}
	return i;
}
