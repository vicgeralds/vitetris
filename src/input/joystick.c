#include <stdio.h>
#include <string.h>
#include "joystick.h"
#include "input.h"
#include "../timer.h"

int num_joyst = 0;
int js_dead = 1;

static struct btns {
	char menu_btns[7];
	char ingame_btns[5];
} btns_joyst[2];

static const char *getbtn(int b, const char *p, int n)
{
	while (n) {
		if (*p == b)
			return p;
		p++;
		n--;
	}
	return (char *) 0;
}

static char *getbtnfor(struct btns *js, int keypress)
{
	char *btns;
	int i;
	if (keypress & IN_GAME) {
		btns = js->ingame_btns;
		if (keypress == (HARDDROP | IN_GAME))
			i = 4;
		else
			i = (keypress & 7)-MVUP;
	} else {
		btns = js->menu_btns;
		if (keypress == STARTBTN)
			i = 6;
		else
			i = (keypress & 7)-MVLEFT;
	}
	return btns+i;
}

static int transl_btn(struct btns *js, int b, int flags)
{
	const char *btns, *p;
	if (flags & IN_GAME) {
		btns = js->ingame_btns;
		if (p = getbtn(b, btns, 5))
			return p==btns+4 ? HARDDROP : MVUP+p-btns;
	}
	btns = js->menu_btns;
	if (p = getbtn(b, btns, 7))
		return p==btns+6 ? STARTBTN : MVLEFT+p-btns;
	return 0;
}

static int test_up_rotate(struct btns *js, int b, int keypr)
{
	if (b==MVUP && keypr==MVUP) {
		b = *getbtnfor(js, MVUP | IN_GAME);
		if (b != MVUP)
			return 0;
	}
	return 1;
}

int js_getpress(int i, int flags)
{
	struct btns *js = btns_joyst+i;
	int b, ret;
	int ingame = flags & IN_GAME;
next:	b = js_readbtn(i);
	if (!b)
		return 0;
	js_dead = 0;
	ret = transl_btn(js, b & 0x7F, flags);
	if (!ret) {
		if (ingame)
			goto next;
		return '.';
	}
	if (b > MVDOWN && (ret <= MVRIGHT || !ingame && ret <= MVDOWN)) {
		js_pressbtn(i, b);
		js_releasebtn(i, *getbtnfor(js, (ret%4)%2 ? ret+1 : ret-1));
	} else if (ingame) {
		if (!test_up_rotate(js, b, ret))
			goto next;
	} else if (autorep_a_b_btns && b > MVDOWN &&
		   (ret == A_BTN || ret == B_BTN))
		js_pressbtn(i, b);
	return ret;
}

static int js_getautorepeat(int i, int flags)
{
	struct btns *js = btns_joyst+i;
	short *tm;
	int b = js_autorep(i, &tm);
	if (!b)
		return 0;
	i = transl_btn(js, b, flags);
	if (flags & IN_GAME) {
		if (i==MVDOWN)
			*tm -= DAS_DELAY/2;
		else if (i > MVRIGHT)
			return 0;
	}
	return i;
}

int getautorepeat(int flags)
{
	int keypress = 0;
	int i = 0;
	while (i < num_joyst) {
		if (js_pressed(i)) {
			keypress = js_getautorepeat(i, flags);
			break;
		}
		i++;
	}
	i++;
	if (keypress && inputdevs_player[i] && !(flags & SINGLE_PL))
		keypress |= inputdevs_player[i]==1 ? PLAYER_1 : PLAYER_2;
	return keypress;
}

int test_autorep_tm(short *tm)
{
	int t = *tm;
	t = gettm(t)-t;
	if (t >= DAS_INITIAL_DELAY) {
		*tm = gettm(0)-t+DAS_DELAY+10000;
		return 1;
	}
	return 0;
}

static void rmbtn(struct btns *js, int b, int keypress)
{
	int flags = keypress & IN_GAME;
	int old = transl_btn(js, b, flags);
	if (old && old != (keypress & 63)) {
		if (flags) {
			old |= flags;
			if (b != *getbtnfor(js, old))
				return;
		}
		js_rmmapping(js!=btns_joyst, old);
	}
}

void js_setmapping(int i, int btn, int keypress)
{
	struct btns *js = btns_joyst+i;
	char *b;
	keypress &= 63 | IN_GAME;
	rmbtn(js, btn, keypress);
	b = getbtnfor(js, keypress);
	if (*b && !(keypress & IN_GAME) && keypress >= MVUP) {
		keypress |= IN_GAME;
		if (*b == *getbtnfor(js, keypress))
			js_setmapping(i, btn, keypress);
	}
	*b = btn;
}

int js_getbtnfor(int i, int keypress)
{
	struct btns *js = btns_joyst+i;
	int b;
	keypress &= 63 | IN_GAME;
	b = *getbtnfor(js, keypress);
	if (b)
		return b;
	if ((keypress & 63) == HARDDROP || keypress == (MVUP | IN_GAME))
		return 0;
	if (keypress & IN_GAME) {
		b = js_getbtnfor(i, keypress ^ IN_GAME);
		if (!b || getbtn(b, js->ingame_btns, 5))
			return 0;
		return b;
	}
	return 0;
}

void js_rmmapping(int i, int keypress)
{
	char *b = getbtnfor(btns_joyst+i, keypress & (63 | IN_GAME));
	if (*b)
		*b = 0;
}

void js_setifnull(int i, int btn, int keypress)
{
	if (!js_getbtnfor(i, keypress))
		js_setmapping(i, btn, keypress);
}

void js_default_buttons(int i)
{
	char *btns = btns_joyst[i].menu_btns;
	char defaults[6] = {MVLEFT, MVRIGHT, MVUP, MVDOWN, '0', '1'};
	if (memchr(btns, '0', 6))
		defaults[4] = 0;
	if (memchr(btns, '1', 6))
		defaults[5] = 0;
	for (i=0; i < 6; i++)
		if (!btns[i])
			btns[i] = defaults[i];
}

const char *js_btnname(int btn)
{
	static char name[4];
	if (btn < '0')
		return (const char *) 0;
	btn -= '0';
	if (btn < 26) {
		name[0] = btn+'A';
		name[1] = '\0';
	} else
		sprintf(name, "%d", btn);
	return name;
}
