#include <linux/input.h>
#include <linux/joystick.h>
#include <stdlib.h>	/* atexit */
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "joystick.h"
#include "input.h"
#include "../timer.h"
#include "../options.h"

static struct js {
	uint32_t pressedbtns;
	short pressedbtns_tm[4];
	short axis_tm[2];
	char axis;
} joysticks[2];

static int set_devname(int i, const char *devname)
{
	union val v;
	v.p = devname;
	setoption(i==0 ? "js0" : "js1", "dev", v, 2);
}

int js_open(int i, const char *devname)
{
	char s[16] = "/dev/";
	int fd;
	if (inputdevs_fd[i+1] > 0)
		return 0;
	if (devname[0] != '/') {
		strncpy(s+5, devname, 10);
		devname = s;
	}
       	fd = open(devname, O_RDONLY);
	if (fd == -1) {
		fprintf(stderr, "Could not open ");
		perror(devname);
		return 0;
	}
	inputdevs_fd[i+1] = fd;
	set_devname(i, devname);
	if (num_joyst <= i)
		num_joyst = i+1;
	return 1;
}

#ifndef JSIOCGBTNMAP
#define getbtnmap(fd, js)
#else
static void getbtnmap(int fd, int i)
{
	unsigned char n;
	uint16_t btnmap[KEY_MAX-BTN_MISC+1];

	if (ioctl(fd, JSIOCGBUTTONS, &n) == -1 || 
	    ioctl(fd, JSIOCGBTNMAP, btnmap) == -1 ||
	    btnmap[0] == BTN_JOYSTICK)
		return;
	while (n) {
		switch (btnmap[--n]) {
		case BTN_A:
			js_setifnull(i, n+'0', A_BTN);
			break;
		case BTN_B:
			js_setifnull(i, n+'0', B_BTN);
			break;
		case BTN_START:
			js_setifnull(i, n+'0', STARTBTN);
		}
	}
}
#endif

static int initjs(int fd, int i)
{
	struct js_event e;
	char name[32] = "";
	int v;
	if (read(fd, &e, sizeof(struct js_event)) < sizeof(struct js_event) ||
	    !(e.type & JS_EVENT_INIT) || ioctl(fd, JSIOCGVERSION, &v) == -1)
		return 0;
	inputdevs_fd[i+1] = fd;
	ioctl(fd, JSIOCGNAME(32), name);
	if (!strcmp(name, "Smartjoy Family Super Smartjoy"))
		js_setifnull(i, 11+'0', STARTBTN);
	getbtnmap(fd, i);
	js_default_buttons(i);
	return 1;
}

static int openjs(const char *devname, int i)
{
	int fd = open(devname, O_RDONLY);
	if (fd > -1) {
		if (initjs(fd, i))
			return 1;
		fprintf(stderr, "Error reading joystick device %s\n", devname);
		close(fd);
	}
	return 0;
}

static void try_openjs()
{
	char devnames[24] = "/dev/js0\0/dev/input/js0";
	char *p;
	int i = 0;
	while (1) {
		if (inputdevs_fd[i+1] == -1) {
			p = devnames;
loop:			if (openjs(p, i))
				set_devname(i, p);
			else if (p == devnames) {
				p += 9;
				goto loop;
			}
		}
		if (i)
			break;
		i = 1;
		devnames[7] = '1';
		devnames[22] = '1';
	}
}

static void upd_num_joysticks()
{
	if (inputdevs_fd[2] > -1)
		num_joyst = 2;
	else
		num_joyst = inputdevs_fd[1] > -1;
}

static void close_joysticks()
{
	if (inputdevs_fd[1] > -1)
		close(inputdevs_fd[1]);
	if (inputdevs_fd[2] > -1)
		close(inputdevs_fd[2]);
}

void init_joysticks()
{
	const char *js0, *js1;
	int *fd;
	js0 = getopt_str("js0", "dev");
	js1 = getopt_str("js1", "dev");
	if (js0 && inputdevs_fd[1] == -1)
		openjs(js0, 0);
	if (!js1)
		js1 = "";
	else if (inputdevs_fd[2] == -1)
		openjs(js1, 1);
	try_openjs();
	if (js0 && !strcmp(js0, js1)) {
		if (strchr(js0, '0')) {
			freeoptions("js1");
			fd = inputdevs_fd+2;
		} else {
			freeoptions("js0");
			fd = inputdevs_fd+1;
		}
		if (*fd > -1) {
			close(*fd);
			*fd = -1;
		}
	}
	upd_num_joysticks();
	atexit(close_joysticks);
}

static void releasebtn(struct js *js, int b)
{
	uint32_t btns = js->pressedbtns;
	short *tm = js->pressedbtns_tm;
	int i = 0;
	uint32_t mask;
	do {
		if (b == (btns & 0x7F)) {
			memmove(tm+i, tm+i+1, (3-i)*sizeof(short));
			js->pressedbtns &= ~(0x7F << 8*i);
			if (!i)
				js->pressedbtns >>= 8;
			else {
				i *= 8;
				mask = ~0 << i;
				js->pressedbtns &= ~mask;
				js->pressedbtns |= (btns & ~0x7F) << i-8;
			}
		}
		i++;
	} while (btns >>= 8);
}

static int readjs_axis(struct js_event *e, struct js *js)
{
	int axis = 0;
	int i = 0;
	int b = 0;
	switch (e->number) {
	case 1:
		i = 2;
	case 0:
		if (e->value) {
			axis = 1+(e->value > 0);
			if (js->axis & axis<<i)
				return 0;
			js->axis_tm[i>>1] = gettm(0);
			b = MVLEFT+i+axis-1;
		}
		js->axis = (axis<<i | js->axis & 12>>i);
	}
	return b;
}

static int readjs(int fd, struct js *js)
{
	struct js_event e;
	int b;
	if (!waitinput(fd, 0) ||
	    read(fd, &e, sizeof(struct js_event)) < sizeof(struct js_event))
		return 0;
	b = 0;
	if (e.type & JS_EVENT_BUTTON) {
		b = e.number+'0';
		if (!e.value) {
			releasebtn(js, b);
			b = 0;
		}
	} else if (e.type & JS_EVENT_INIT)
		js->axis = 0;
	else if (e.type & JS_EVENT_AXIS)
		b = readjs_axis(&e, js);
	else {
		js->pressedbtns = 0;
		js->axis = 0;
		return -1;
	}
	return b ? b : readjs(fd, js);
}

int js_readbtn(int i)
{
	int fd = inputdevs_fd[i+1];
	int b = readjs(fd, joysticks+i);
	if (b > -1)
		return b;
	close(fd);
	inputdevs_fd[i+1] = -1;
	upd_num_joysticks();
	return 0;
}

void js_pressbtn(int i, int b)
{
	struct js *js = joysticks+i;
	short *tm = js->pressedbtns_tm;
	releasebtn(js, b);
	js->pressedbtns = b | js->pressedbtns << 8;
	tm[3] = tm[2];
	tm[2] = tm[1];
	tm[1] = tm[0];
	tm[0] = gettm(0);
}

void js_releasebtn(int i, int b)
{
	releasebtn(joysticks+i, b);
}

int js_pressed(int i)
{
	struct js *js = joysticks+i;
	return js->axis || js->pressedbtns;
}

static int getautorep_btn(struct js *js, short **tm)
{
	uint32_t btns = js->pressedbtns;
	int i = 0;
	do {
		if (test_autorep_tm(js->pressedbtns_tm+i)) {
			*tm = js->pressedbtns_tm+i;
			return btns & 0x7F;
		}
		i++;
	} while (btns >>= 8);
	return 0;
}

int js_autorep(int i, short **tm_ret)
{
	struct js *js = joysticks+i;
	short *tm = js->axis_tm;
	int b = 0;
	if (js->axis & 3 && test_autorep_tm(tm))
		b = MVLEFT+(js->axis & 3)-1;
	else if (js->axis & 12 && test_autorep_tm(tm+1)) {
		b = MVUP+(js->axis>>2)-1;
		tm++;
	} else if (js->pressedbtns)
		b = getautorep_btn(js, &tm);
	*tm_ret = tm;
	return b;
}

void js_reset_drop(int i)
{
	struct js *js = joysticks+i;
	int t = gettm(0);
	js->axis_tm[1] = t;
	js->pressedbtns_tm[0] = t;
	js->pressedbtns &= 0x7F;
}
