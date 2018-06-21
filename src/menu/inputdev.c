#include <string.h>
#include <ctype.h>
#include "menu.h"
#include "../input/input.h"
#include "../textgfx/textgfx.h"
#include "../options.h"
#undef JOYSTICK
#define JOYSTICK 1
#include "internal.h"

static char devices_str[14];

static void upd_devstr()
{
	strcpy(devices_str, "keybd js0 js1");
	if (num_joyst == 1)
		inputdevs_fd[2] = -1;
	if (num_joyst && (inputdevs_fd[1] > -1 || inputdevs_fd[2] > -1)) {
		if (inputdevs_fd[2] < 0)
			devices_str[9] = '\0';
		if (inputdevs_fd[1] < 0) {
			devices_str[8] = '1';
			devices_str[9] = '\0';
		}
	} else
		devices_str[5] = '\0';
}

static const char *getdevstr(int i)
{
	char *p = devices_str;
	while (i) {
		p = strchr(p, ' ');
		if (!p)
			return NULL;
		p++;
		i--;
	}
	return p;
}

static int getdev(const char *key)
{
	char *p;
	if (key && (p = strchr(devices_str, key[2])) && isdigit(*p))
		return *p-'0'+1;
	return 0;
}

static const char *player_devstr(const char **sect_name)
{
	const char *s = "";
	const char *dev;
	if (player_)
		s = (player_==1) ? "player1" : "player2";
	dev = getopt_str(s, "input");
	if (sect_name)
		*sect_name = s;
	return dev;
}

static int inp_devhandler(int keypr, int *pos)
{
	const char *sect_name;
	const char *dev = player_devstr(&sect_name);
	const char *dev2;
	int i = 0;
	union val v;
	upd_devstr();
	if (dev) {
		do i++;
		while ((dev2 = getdevstr(i)) && strncmp(dev, dev2, 3));
		if (!dev2)
			i = 0;
		else if (player_) {
			v.integ = inputdevs_player[getdev(dev)];
			if (v.integ && v.integ != player_)
				i = 0;
		}
	}
	switch (keypr) {
	case 0:
		break;
	case MVLEFT:
		i--; break;
	case MVRIGHT:
		i++; break;
	case MVDOWN:
	case '\t':
		*pos = 2;
		return 2;
	default:
		return 0;
	}
	dev = getdevstr(i);
	if (!dev)
		return 1;
	printmenuitem_options(devices_str, i);
	if (!i)
		unsetoption(sect_name, "input");
	else {
		strncpy(v.str, dev, 4);
		setoption(sect_name, "input", v, 1);
	}
#if TWOPLAYER
	if (player_) {
		i = getdev(dev);
		inputdevs_player[i] = player_;
		if (!i) {
			sect_name = player_==1 ? "player2" : "player1";
			if (!getdev(getopt_str(sect_name, "input")))
				inputdevs_player[0] = 0;
		}
	}
#endif
	if (keypr) {
		*pos = 1;
		return 2;
	}
	return 1;
}

int inp_devlist(int *dev, int x, int y)
{
	const char *device = "Device";
	menuhandler handler = inp_devhandler;
	int ret = 0;
	drawmenu(&device, 1, 0, x, y, &handler);
	while (1) {
		*dev = getdev(player_devstr(NULL));
		if (ret == 3)
			break;
		inp_printkeys(*dev, x, y+1);
		ret = openmenu(&device, 1, 0, x, y, &handler);
		if (ret < 2)
			return 0;
	}
	setcurs(x, y);
	printmenuitem(device, 0);
	newln(x);
	return 1;
}

#ifndef NO_MENU
int inputsetup_menuitem(int k, int *pos)
{
	int i = 0;
	if (!num_joyst) {
		i = rarrow_menuitem(k, pos);
		cleartoeol();
		return i;
	}
	if (inp_devhandler(k, &i)) {
		if (k && i!=1)
			movefwd(strlen(devices_str));
		printstr(" ->");
		if (i==1)
			return 3;
		return k <= MVRIGHT;
	}
	return 0;
}
#endif
