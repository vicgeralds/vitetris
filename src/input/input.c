#include <string.h>
#include <stdio.h>	/* sprintf */
#include <stdlib.h>	/* exit */
#include "input.h"
#include "keyboard.h"
#include "joystick.h"
#include "../timer.h"
#include "../draw/draw.h"  /* upd_screen */
#include "../game/tetris.h"
#ifndef SOCKET
#define SOCKET_EMPTY_DEFS 1
#undef socket_fd
#define socket_fd -1
#endif
#include "../netw/sock.h"

#include "../textgfx/textgfx.h" /* upd_termresize */
#include "../focus.h"

extern int close_button_pressed;

const char input_chr9[9][4] = {
	"LFT", "RGT", "UP", "DWN", "(A)", "(B)", "BEL", "BS", "HT"
};

int inputdevs_fd[4] = {-1,-1,-1,-1};
char inputdevs_player[4] = {0};

int autorep_a_b_btns = 0;
int edit_mode = 0;

static short spawn_tm[2];
static int ignore_dev;

void init_inputdevs()
{
	init_keybd();
#if JOYSTICK
	init_joysticks();
#endif
}

#if TWOPLAYER && NUM_INPUTDEVS > 1
static int player_flag(int i, int flags)
{
	if (socket_fd > -1) {
		if (i==3)
			return PLAYER_2;
	} else if (inputdevs_player[i] && !(flags & SINGLE_PL))
		return inputdevs_player[i]==1 ? PLAYER_1 : PLAYER_2;
	return 0;
}
#else
#define player_flag(i, flags) 0
#endif

static int getkeypress_select(int tm, int flags)
{
	int keypress = 0;
	int i;
#if TERM_RESIZING
	if (tm > 80)
		upd_termresize();
#endif
#if XLIB || ALLEGRO
	if (tm > 80 && in_xterm && game && game_running &&
	    !TWOPLAYER_MODE && !xterm_hasfocus())
		return STARTBTN;
#endif
	for (i = inpselect_dev(tm); i < NUM_INPUTDEVS && !keypress; i++) {
		if (i+1 == ignore_dev)
			continue;
		switch (i) {
		case 0:
			keypress = kb_getpress(flags);
			break;
#if JOYSTICK
		case 1:
		case 2:
			keypress = js_getpress(i-1, flags);
			break;
#endif
#if SOCKET
		case 3:
			keypress = sock_getkeypress(flags);
			if (ignore_dev)
				return 0;
#endif
		}
	}
	if (keypress)
		keypress |= player_flag(i-1, flags);
	return keypress;
}

int getkeypress(int tm, int flags)
{
	int t = gettm(0);
	int keypress;
	if (socket_fd > -1)
		flags |= SINGLE_PL;
	while (1) {
		if (tm <= 0)
			return 0;
		keypress = getkeypress_select(tm, flags);
#if JOYSTICK
		if (!keypress)
			keypress = getautorepeat(flags);
#endif
		if (keypress)
			break;
		if (gettm(t) == t)
			sleep_msec(1);
		tm -= gettm(t)-t;
		t = gettm(0);
#ifdef ALLEGRO
		if (close_button_pressed)
			exit(0);
#endif
	}
	return keypress;
}

int getkeypress_block(int flags)
{
	int keypress;
	while (1) {
#if TTY_SOCKET && !NO_MENU
		if (!(flags & 1) && (invit || checkinvit()))
			return ESC;
#endif
		keypress = getkeypress(1000, flags);
		if (keypress)
			break;
		sleep_msec(55);
	}
	return keypress;
}

void spawn_discard_drops(int pl)
{
	int i = 0;
	if (pl==2) {
		i = 1;
		if (socket_fd > -1) {
			spawn_tm[1] = 0;
			return;
		}
	}
	spawn_tm[i] = gettm(0);
#ifdef ALLEGRO
	kb_reset_drop(pl);
#endif
#ifdef JOYSTICK
	if (!num_joyst)
		return;
	i = !js_pressed(0);
	if (pl==2 && inputdevs_player[i+1] != 2) {
		if (i || inputdevs_player[2] != 2)
			return;
		i = 1;
	}
	if (js_pressed(i))
		js_reset_drop(i);
#endif
}

static int key_is_valid(unsigned c, int keypr)
{
	return c == (keypr & 0x7F) ||
		c > MVRIGHT &&
		(c > MVDOWN || keypr & IN_GAME) &&
		c != STARTBTN && c != '\t';
}

static int setkeymapping_keybd(int keypr)
{
	unsigned char s[5] = "";
	unsigned c;
	kb_no_autorep = 1;
	while (!kb_readkey(s)) {
		if (c = getkeypress_select(100, SINGLE_PL))
			return c;
		sleep_msec(55);
	}
	c = !s[1] ? s[0] : ESC+1;
	if (c == ESC)
		kb_rmmapping(keypr);
	else {
		if (!key_is_valid(c, keypr))
			return c;
		if (c == MVUP && (keypr & (63 | IN_GAME)) == MVUP) {
			c = kb_getchrfor(keypr);
			if (c == MVUP)
				return MVUP;
		}
		kb_setmapping(s, keypr);
		if (!c && (!(keypr & PLAYER_2) || inputdevs_player[0]==2))
			return MVUP;
	}
	return MVDOWN;
}

#if JOYSTICK
static int setkeymapping_js(int i, int keypr)
{
	int b, b2;
	while (!(b = js_readbtn(i))) {
		if (inputdevs_fd[i+1] == -1)
			return STARTBTN;
		if (b = getkeypress_select(100, SINGLE_PL)) {
			if (b == ESC) {
				js_rmmapping(i, keypr);
				return MVDOWN;
			}
			return b;
		}
		sleep_msec(55);
	}
	b2 = js_readbtn(i);
	if (b > b2 && b2)
		b = b2;
	if (!key_is_valid(b, keypr))
		return b;
	if (b == MVUP && keypr == MVUP && js_getbtnfor(i, MVUP) == MVUP)
		return MVUP;
	js_setmapping(i, b, keypr);
	return MVDOWN;
}
#endif

int setkeymapping(int dev, int keypr)
{
	ignore_dev = dev+1;
#if JOYSTICK
	if (dev > 0)
		keypr = setkeymapping_js(dev-1, keypr);
	else
#endif
		keypr = setkeymapping_keybd(keypr);
	kb_no_autorep = 0;
	ignore_dev = 0;
	return keypr;
}

static const char *getkeyfor_str_keybd(int keypr, int *c)
{
	static unsigned char bytes[7];
	const char *name;
	int n = kb_getkeyfor(keypr, bytes, 1);
	if (!n)
		return "";
	name = kb_keyname(bytes, n);
	if (name)
		return name;
	bytes[n] = '\0';
	if (n==1)  {
		n = bytes[0];
		if (n ==' ')
			strcpy((char *) bytes, "SPACE");
		else if (n == DEL)
			strcpy((char *) bytes, "DEL");
		else if (n < ' ') {
			if (n < 10) {
				*c = n;
				return NULL;
			}
			sprintf((char *) bytes, "%d", n);
		}
	}
	return (const char *) bytes;
}

const char *getkeyfor_str(int dev, int keypr)
{
	const char *name;
	int c = 0;
#if JOYSTICK
	if (dev) {
		c = js_getbtnfor(dev-1, keypr);
		name = js_btnname(c);
	} else
#endif
		name = getkeyfor_str_keybd(keypr, &c);
	if (name)
		return name;
	if (c > 0 && c < 10)
		return input_chr9[c-1];
	return "";
}

static int dropsafe(int i)
{
	int t = spawn_tm[i];
	if (t && gettm(t)-t < DAS_DELAY)
		return 1;
	spawn_tm[i] = 0;
	return 0;
}

int processkey_ingame(int key, int flags)
{
	static int discard_count;
	struct player *plr;
	int i = TWOPLAYER_MODE && key & PLAYER_2;
	int safe;
	switch (key & 0x7F) {
	case ESC:
	case '\b':
		game->state = 0;
		return -2;
	case 'q':
		exit(0);
	case STARTBTN:
	case 'p':
		if (flags & NO_PAUSE || !game_running || TWOPLAYER_MODE)
			break;
		i = pausegame();
		textgfx_flags &= ~LOST_FOCUS;
		return i;
	}
	if (flags & DISCARD_MOVES) {
		if (++discard_count > 5)
			kb_flushinp();
		return -1;
	}
	discard_count = 0;
	plr = game->player+i;
	safe = dropsafe(i);
	key &= 0x7F;
	if (!(flags & DISCARD_DROPS)) {
		switch (key) {
		case HARDDROP:
			if (socket_fd > -1 && !i) {
				sock_sendpiece(plr);
				if (harddrop(plr, safe) == -1)
					return -1;
				sock_sendbyte(HARDDROP);
				return 0;
			}
			return harddrop(plr, safe);
		case MVDOWN:
			if (!TWOPLAYER_MODE)
				return softdrop(softdrop_speed, safe);
			if (!movedown(plr, 1))
				return 0;
			if (socket_fd > -1 && !i)
				sock_sendbyte(MVDOWN);
			return 1;
		}
	}
	switch (key) {
	case MVLEFT:
		moveleft(plr);
		break;
	case MVRIGHT:
		moveright(plr);
		break;
	case MVUP:
		rotate(plr, plr->rotationsys & ROT_CLOCKWISE);
		break;
	case A_BTN:
		rotate(plr, 1);
		break;
	case B_BTN:
		rotate(plr, 0);
		break;
	default:
		return -1;
	}
	if (socket_fd > -1 && !i)
		sock_sendbyte(key);
	upd_screen(1+i);
	return 1;
}
