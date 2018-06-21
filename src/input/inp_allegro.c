#include <string.h>
#include <stdio.h>	/* sprintf */
#include "../config.h"
#include <allegro.h>
#include "keyboard.h"
#include "joystick.h"
#include "input.h"
#include "../timer.h"
#include "../game/tetris.h"
#include "../textgfx/textgfx.h" /* refreshscreen */
#include "../textgfx/alleg.h"	/* toggle_fullscreen */

static short key_tm[KEY_MAX];

static int temp_autorep_key;

static char arrow_keys[8] = {KEY_LEFT, KEY_RIGHT, KEY_UP, KEY_DOWN};

static struct js {
	short btns_tm[32];
	short axis_tm[2];
	char axis[2];
} joysticks[2];

void init_keybd()
{
	install_keyboard();
	set_keyboard_rate(0, 0);
}

void init_joysticks()
{
	install_joystick(JOY_TYPE_AUTODETECT);
	if (num_joysticks > 0) {
		inputdevs_fd[1] = 1;
		js_default_buttons(0);
		num_joyst = 1;
		if (num_joysticks > 1) {
			inputdevs_fd[2] = 1;
			js_default_buttons(1);
			num_joyst = 2;
		}
	}
}

static int readshiftkey()
{
	const shift_keys[6] = {
		KEY_LSHIFT, KEY_RSHIFT,
		KEY_LCONTROL, KEY_RCONTROL,
		KEY_ALT, KEY_ALTGR
	};
	int scan;
	int i;
	for (i=0; i < 6; i++) {
		scan = shift_keys[i];
		if (key[scan] && !key_tm[scan])
			return scan;
	}
	return 0;
}

static void update_key_tm(int i)
{
	char *p;
	int tm;
	if (i) {
		if (edit_mode)
			memset(key_tm, 0, KEY_MAX * sizeof(short));
		key_tm[i] = gettm(0);
		p = memchr(arrow_keys, i, 8);
		if (p) {
			i = p - arrow_keys;
			if (i%4 < 2 || !game || !game_running) {
				if ((i%4)%2 == 0)
					i++;
				else
					i--;
				key_tm[arrow_keys[i]] = -1;
			}
		}
	}
	for (i=1; i < KEY_MAX; i++)
		if (key_tm[i] && !key[i])
			key_tm[i] = 0;
}

static void setarrowkey(int scan, int press)
{
	char *p = arrow_keys;
	if (press & PLAYER_2)
		p += 4;
	p[(press & 7)-1] = scan;
}

static int scan_to_ascii(int scan)
{
	if (scan >= KEY_0_PAD && scan <= KEY_9_PAD)
		return 0;
	switch (scan) {
	case KEY_BACKSPACE: return '\b';
	case KEY_DEL:       return DEL;
	}
	return scancode_to_ascii(scan);
}

static int getautorep_key(int *ascii)
{
	int flags = 0;
	int press;
	int i;
	for (i=1; i < KEY_MAX; i++) {
		if (key_tm[i] > 0 && key[i] && test_autorep_tm(key_tm+i)) {
			*ascii = scan_to_ascii(i);
			break;
		}
	}
	if (i == KEY_TAB)
		return KEY_TAB;
	if (i == KEY_MAX || kb_no_autorep)
		return 0;
	if (edit_mode && (i == KEY_BACKSPACE || i == KEY_DEL))
		return i;
	if (game && game_running)
		flags |= IN_GAME;
	if (!game || (game->mode&(MODE_2PLAYER|MODE_NETWORK)) != MODE_2PLAYER)
		flags |= SINGLE_PL;
	temp_autorep_key = i<<8 | *ascii;
	press = kb_getpress(flags);
	temp_autorep_key = 0;
	if (!press)
		return 0;
	if ((press & 0x7F) <= MVRIGHT) {
		setarrowkey(i, press);
		return i;
	}
	if (flags & IN_GAME) {
		if ((press & 0x7F) == MVDOWN) {
			key_tm[i] -= DAS_DELAY/2;
			return i;
		}
	} else if ((press & 0x7F) <= MVDOWN) {
		setarrowkey(i, press);
		return i;
	} else if (autorep_a_b_btns)
		switch (press & 0x7F) {
		case A_BTN:
		case B_BTN:
			return i;
		}
	return 0;
}

int kb_readkey(unsigned char *dest)
{
	int val = 0;
	int scan;
	if (temp_autorep_key) {
		val = temp_autorep_key;
		scan = val >> 8;
		temp_autorep_key = 1;
	} else {
		refreshscreen();
		if (keypressed()) {
			val = readkey();
			scan = val >> 8;
			if (scan == KEY_ENTER && key_shifts & KB_ALT_FLAG) {
				toggle_fullscreen();
				kb_flushinp();
				return 0;
			}
		} else
			scan = readshiftkey();
		update_key_tm(scan);
		if (!scan)
			scan = getautorep_key(&val);
	}
	if (!scan)
		return 0;
	if (scan >= KEY_A && scan <= KEY_Z && !edit_mode) {
		val = scan+'a'-KEY_A;		/* caps lock ignored */
		if (key_shifts & KB_SHIFT_FLAG)
			val -= 'a'-'A';
	}
	if (scan >= KEY_LEFT && scan <= KEY_DOWN)
		val = scan+MVLEFT-KEY_LEFT;
	else if (scan == KEY_DEL)
		val = DEL;
	else if (scan >= KEY_0_PAD && scan <= KEY_9_PAD)
		val = 0;
	else
		val &= 0xFF;
	if (val) {
		if (val == '\r')
			val = '\n';
		dest[0] = val;
		return 1;
	}
	dest[0] = 0;
	dest[1] = scan;
	return 2;
}

int kb_toascii(const unsigned char *key)
{
	int scan = key[1];
	if (scan >= KEY_0_PAD && scan <= KEY_9_PAD)
		return scan-KEY_0_PAD+'0';
	return ESC+1;
}

static int key_is_softdrop(int k, int pl)
{
	int pl2;
	temp_autorep_key = k<<8 | scan_to_ascii(k);
	k = kb_getpress(IN_GAME);
	temp_autorep_key = 0;
	pl2 = k & PLAYER_2;
	if (pl==1 && pl2 || pl==2 && !pl2)
		return 0;
	return (k & 0x7F) == MVDOWN;
}

void kb_reset_drop(int pl)
{
	int i;
	if (pl && inputdevs_player[0]) {
		if (pl != inputdevs_player[0])
			return;
		pl = 0;
	}
	for (i=1; i < KEY_MAX; i++)
		if (key_tm[i] > 0 && key_is_softdrop(i, pl))
			key_tm[i] = gettm(0);
}

void kb_flushinp()
{
	clear_keybuf();
	memset(key_tm, 0, KEY_MAX * sizeof(short));
	temp_autorep_key = 0;
}

const char *kb_keyname(unsigned char *key, int n)
{
	static char numstr[6] = "NUM  ";
	int scan;
	const char *name;
	if (n < 2)
		return NULL;
	scan = key[1];
	switch (scan) {
	case KEY_LSHIFT:   return "LSHFT";
	case KEY_RSHIFT:   return "RSHFT";
	case KEY_LCONTROL: return "LCTRL";
	case KEY_RCONTROL: return "RCTRL";
	case KEY_ALT:      return "ALT";
	case KEY_ALTGR:    return "ALTGR";
	}
	if (scan >= KEY_0_PAD && scan <= KEY_9_PAD) {
		numstr[4] = scan-KEY_0_PAD+'0';
		return numstr;
	}
	name = scancode_to_name(key[1]);
	if (strlen(name) <= 5)
		return name;
	sprintf(key, "0+%X", key[1]);
	return key;
}

static void js_update()
{
	int n;
	poll_joystick();
	n = num_joysticks;
	if (n > 2)
		n = 2;
	if (n != num_joyst) {
		num_joyst = n;
		inputdevs_fd[1] = -1;
		inputdevs_fd[2] = -1;
		if (n > 0) {
			inputdevs_fd[1] = 1;
			if (n > 1)
				inputdevs_fd[2] = 1;
		}
	}
}

static int readaxis(int i, int j)
{
	int axis = 0;
	int b = 0;
	if (joy[i].stick[0].axis[j].d1 == joy[i].stick[0].axis[j].d2)
		;
	else if (joy[i].stick[0].axis[j].d1) {
		axis = -1;
		b = MVLEFT;
	} else {
		axis = 1;
		b = MVRIGHT;
	}
	if (axis == joysticks[i].axis[j])
		return 0;
	joysticks[i].axis[j] = axis;
	if (!axis)
		return 0;
	joysticks[i].axis_tm[j] = gettm(0);
	if (j)
		b += 2;
	return b;
}

int js_readbtn(int i)
{
	int a, b;
	js_update();
	if (i >= num_joyst)
		return 0;
	b = 0;
	a = readaxis(i, 0);
	if (a)
		b = a;
	a = readaxis(i, 1);
	if (a)
		b = a;
	for (a=0; a < 32 && a < joy[i].num_buttons; a++) {
		if (!joy[i].button[a].b)
			joysticks[i].btns_tm[a] = 0;
		else if (!b && !joysticks[i].btns_tm[a]) {
			b = a+'0';
			joysticks[i].btns_tm[a] = -1;
		}
	}
	return b;
}

void js_pressbtn(int i, int b)
{
	joysticks[i].btns_tm[b-'0'] = gettm(0);
}

void js_releasebtn(int i, int b)
{
	joysticks[i].btns_tm[b-'0'] = -1;
}

int js_pressed(int i)
{
	struct js *js = joysticks+i;
	int n = 32;
	if (i < num_joysticks) {
		if (js->axis[0] || js->axis[1])
			return 1;
		if (n > joy[i].num_buttons)
			n = joy[i].num_buttons;
		for (i=0; i < n; i++)
			if (js->btns_tm[i] > 0)
				return 1;
	}
	return 0;
}

int js_autorep(int i, short **tm_ret)
{
	struct js *js = joysticks+i;
	int n = 32;
	if (js->axis[0] && test_autorep_tm(js->axis_tm)) {
		*tm_ret = js->axis_tm;
		return MVLEFT+(js->axis[0] > 0);
	}
	if (js->axis[1] && test_autorep_tm(js->axis_tm+1)) {
		*tm_ret = js->axis_tm+1;
		return MVUP + (js->axis[1] > 0);
	}
	if (n > joy[i].num_buttons)
		n = joy[i].num_buttons;
	for (i=0; i < n; i++)
		if (js->btns_tm[i] > 0 && test_autorep_tm(js->btns_tm+i)) {
			*tm_ret = js->btns_tm+i;
			return i+'0';
		}
	return 0;
}

void js_reset_drop(int i)
{
	struct js *js = joysticks+i;
	int n = 32;
	js->axis_tm[1] = gettm(0);
	if (n > joy[i].num_buttons)
		n = joy[i].num_buttons;
	for (i=0; i < n; i++)
		if (js->btns_tm[i] > 0)
			js->btns_tm[i] = gettm(0);
}

int inpselect_dev(int tm)
{
	return 0;
}
