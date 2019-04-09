/* Startup and game menus */

#include "menu.h"
#include "../input/input.h"
#include "../textgfx/textgfx.h"
#include "../draw.h"
#include "../options.h"
#include "../game/tetris.h"
#include "../lang.h"

#ifdef JOYSTICK
int inputsetup_menuitem(int k, int *p);
#else
#define inputsetup_menuitem rarrow_menuitem
#endif

#ifdef SOCKET
int gamemenu_socket(const char **menu, int i, int x, int y, menuhandler *hs);
#else
#define gamemenu_socket(menu, i, x, y, handlers) 0
#endif

static const char *modes[3] = {"A-type", "B-type", "40 lines"};
static int gm_rot_state[2];

static int gamemenu_2p(const char **menu, int x, int y, menuhandler *hs);

static int gm_mode(int k, int *pos);
static int gm_level(int k, int *pos);
static int gm_height(int k, int *pos);
static int gm_rotation(int k, int *pos);
static int gm_softdrop(int k, int *pos);

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
	menuhandler handlers[7] = {NULL, NULL,
#ifdef INET
				NULL,
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
			v.integ = MODE_2P | MODE_NET;
		else
#endif
		v.integ = i;
		v.integ |= getopt_int("", "mode") & MODE_LINECLEAR;
		setoption("", "mode", v, 0);
	}
	return i;
}

int game_menu(int i, int x, int y)
{
	const char *menu[GAMEMENU_LENGTH] = {
		"Level",
		"Height",
		"Input Setup",
		"Rotation",
		"Softdrop"
	};
	menuhandler handlers[GAMEMENU_LENGTH] = {
		gm_level, gm_height, inputsetup_menuitem,
		gm_rotation, gm_softdrop
	};
	struct player *p = &player1;
	const char *s = "";
	union val v;
	int mode2p = 0;

	player_ = 0;
	gm_rot_state[0] = 0;
	gm_rot_state[1] = 0;
	if (socket_fd > -1)
		i = gamemenu_socket(menu, i, x, y, handlers);
	else if (game.mode & MODE_2P) {
		if (game.mode & MODE_NET)
			return 0;
		mode2p = 1;
		i = gamemenu_2p(menu, x, y, handlers);
	} else
		i = openmenu(menu, GAMEMENU_LENGTH, i, x, y, handlers);
	if (i==3)
		return 3;
	if (!i)
		return 0;
	if (mode2p)
		s = "player1";
	while (1) {
		v.integ = p->startlevel;
		setoption(s, "level", v, 0);
		v.integ = p->height;
		setoption(s, "height", v, 0);
		v.integ = p->rotation;
		setoption(s, "rotate", v, 0);
		if (!(game.mode & MODE_2P)) {
			v.integ = softdrop_speed;
			setoption("", "softdrop", v, 0);
		}
		if (!mode2p || p == &player2)
			break;
		p = &player2;
		s = "player2";
	}
	return 1;
}

static int gamemenu_2p(const char **menu, int x, int y, menuhandler *handlers)
{
	int y2 = y+2+GAMEMENU_LENGTH;
	int yy = y+1;
	int keypr;
	int i1 = 0;
	int i2 = 0;
	int *i = &i1;
begin:	setcurs(x, y);
	printstr("PLAYER 1");
	setcurs(x, y+GAMEMENU_LENGTH+1);
	printstr("PLAYER 2");
back:	player_ = 1;
	drawmenu(menu, GAMEMENU_LENGTH-1, i1, x, y+1, handlers);
	player_ = 2;
	drawmenu(menu, GAMEMENU_LENGTH-1, i2, x, y2, handlers);
	while (1) {
		setcurs(x, yy+*i);
		refreshwin(-1);
		keypr = getkeypress_block(0);
		if (keypr==STARTBTN && i1<2 && i2>=2)
			keypr |= PLAYER_2;
		else if (inputdevs_player[0]==2 && i1>=2 && i2<2 &&
						keypr==(STARTBTN | PLAYER_2))
			keypr = STARTBTN;
		if (!(keypr & PLAYER_2)) {
			player_ = 1;
			i = &i1;
			yy = y+1;
		} else {
			player_ = 2;
			i = &i2;
			yy = y2;
		}
		setcurs(x, yy+*i);
		switch (handle_menuitem(menu, GAMEMENU_LENGTH-1, i,
					x, yy+*i, handlers, keypr)) {
		case 0:
			return 0;
		case 2:
			if (*i==2) {
				inputsetup_screen(player_, x, y);
				*i = 0;
				goto begin;
			}
			return 1;
		case 3:
			goto back;
		}
	}
}

static int getmodeindex(int mode)
{
	int i = 1-!(mode & MODE_B);
	if (mode & MODE_40L)
		i = 2;
	return i;
}

const char *getmodestr()
{
	return modes[getmodeindex(getopt_int("", "mode"))];
}

/* Menu item handlers */

static int gm_mode(int k, int *pos)
{
	const char **items = modes;
	const char *abbr[3]  = {"A-type", "B-type", "40L"};
	union val v;
	int i;
	int ret;
	int mode = getopt_int("", "mode");
	i = getmodeindex(mode);
	ret = selectitem(items, abbr, 3, &i, k);
	if (ret && k) {
		mode &= ~MODE_LINECLEAR;
		if (i) {
			if (i==1) {
				v.integ = 25;
				mode |= MODE_B;
			} else {
				v.integ = 40;
				mode |= MODE_40L;
			}
			setoption("", "lines", v, 0);
		}
		v.integ = mode;
		setoption("", "mode", v, 0);
	}
	return ret;
}

static int gm_level(int k, int *pos)
{
	struct player *p = game.player+(player_==2);
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
	struct player *p = game.player+(player_==2);
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

static int rotsys(int k, int sel)
{
	const char *items[3] = {"lefthanded (GB)",
				"righthanded (NES)",
				"modern (SRS-like)"};
	const char *abbr[3]  = {"left  ",
				"right ",
				"modern"};
	struct player *p = game.player+(player_==2);
	int i = !(p->rotation & ROT_LEFTHAND);
	int ret;
	if (p->rotation & ROT_MODERN)
		i = 2;
	if (!sel) {
		putch(' ');
		printstr(abbr[i]);
		printstr("  ");
		return 0;
	}
	ret = selectitem(items, abbr, 3, &i, k);
	if (ret && k) {
		p->rotation &= ~(ROT_LEFTHAND | ROT_MODERN);
		if (i==0)
			p->rotation |= ROT_LEFTHAND;
		if (i==2)
			p->rotation |= ROT_MODERN;
	}
	return ret;
}

static int rotdir(int k, int sel, int x, int y)
{
	char anticlockwise[17] = "anticlockwise";
	char acw[] = "acw";
	const char *items[2] = {"clockwise", anticlockwise};
	const char *abbr[2]  = {"cw ", acw};
	struct player *p = game.player+(player_==2);
	int i = !(p->rotation & ROT_CLOCKWISE);
	int ret;
	spellword(anticlockwise);
	spellword(acw);
	setcurs(x+9, y);
	if (!sel) {
		putch(' ');
		printstr(abbr[i]);
		putch(' ');
		return 0;
	}
	ret = selectitem(items, abbr, 2, &i, k);
	if (ret && k) {
		p->rotation &= ~ROT_CLOCKWISE;
		if (i==0)
			p->rotation |= ROT_CLOCKWISE;
	}
	return ret;
}

static int gm_rotation(int k, int *pos)
{
	int i = gm_rot_state[player_==2];
	int x, y;
	int ret;
	if (!i && (k==MVDOWN || k=='\t')) {
		i++;
		k = 0;
	} else if (i && k==MVUP) {
		i--;
		k = 0;
	}
	get_xy(&x, &y);
	if (!i) {
		ret = rotsys(k, 1);
		rotdir(0, 0, x, y);
	} else {
		rotsys(0, 0);
		ret = rotdir(k, 1, x, y);
	}
	if (!k) {
		gm_rot_state[player_==2] = i;
		ret = 1;
	}
	return ret;
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
