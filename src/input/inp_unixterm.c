#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>	/* atexit */
#include <string.h>	/* strlen */
#include "select.h"
#include "termin.h"
#include "keyboard.h"
#include "escseq.h"
#include "input.h"
#include "joystick.h"	/* js_pressed */

#ifdef JOYSTICK
#define JS_PRESSED (js_pressed(0) || js_pressed(1))
#else
#define JS_PRESSED 0
#endif

char unreadchr = '\0';

static struct termios saved_term_attr;

void set_input_mode()
{
	struct termios attr;
	tcgetattr(STDIN_FILENO, &saved_term_attr);
	tcgetattr(STDIN_FILENO, &attr);
	attr.c_lflag &= ~(ICANON|ECHO);
	attr.c_cc[VMIN] = 0;
	attr.c_cc[VTIME] = 2;
	tcsetattr(STDIN_FILENO, TCSADRAIN, &attr);
	atexit(free_escape_sequences);
}

void restore_input_mode()
{
	tcsetattr(STDIN_FILENO, TCSANOW, &saved_term_attr);
}

int readchr()
{
	int ret;
	if (!unreadchr) {
		if (!waitinput_stdin(0))
			return 0;
		ret = read(STDIN_FILENO, &unreadchr, 1);
		if (ret <= 0)
			return 0;
	}
	ret = (unsigned char) unreadchr;
	unreadchr = '\0';
	return ret;
}

int waitinput_stdin(unsigned msec)
{
	return waitinput(STDIN_FILENO, msec);
}

void init_keybd()
{
	inputdevs_fd[0] = STDIN_FILENO;
}

int kb_readkey(unsigned char *dest)
{
	int c = readchr();
	if (!c)
		return 0;
	if (c == ESC)
		switch (c = readescseq((char *) dest+1)) {
		case -1:
			return 0;
		case 0:
			dest[0] = ESC;
			return strlen((char *) dest);
		}
	else if (c == DEL)
		c = BACKSPACE;
	*dest = c;
	return 1;
}

int kb_toascii(const unsigned char *key)
{
	return ESC+1;
}

void kb_flushinp()
{
	while (readchr())
		;
}

const char *kb_keyname(unsigned char *key, int n)
{
	if (n < 2)
		return NULL;
	memmove(key+3, key+1, n);
	memcpy(key, "ESC", 3);
	return (const char *) key;
}

static void settimeout(struct timeval *tmv, int ms)
{
	if (ms > 5 && JS_PRESSED) {
		tmv->tv_sec = 0;
		tmv->tv_usec = 5000;
	} else {
		tmv->tv_sec = ms/1000;
		tmv->tv_usec = 1000*(ms%1000);
	}
}

static int setfds(fd_set *set)
{
#if NUM_INPUTDEVS > 1
	int n = 0;
	int i = NUM_INPUTDEVS-1;
	FD_ZERO(set);
	while (i >= 0) {
		if (inputdevs_fd[i] > -1) {
			FD_SET(inputdevs_fd[i], set);
			if (inputdevs_fd[i] >= n)
				n = inputdevs_fd[i]+1;
		}
		i--;
	}
	return n;
#else
	FD_ZERO(set);
	FD_SET(STDIN_FILENO, set);
	return STDIN_FILENO+1;
#endif
}

int inpselect_dev(int tm)
{
	fd_set set;
	struct timeval tmv;
	int i;
	settimeout(&tmv, tm);
	if (inpselect(setfds(&set), &set, &tmv) <= 0)
		return NUM_INPUTDEVS;
	for (i=0; i < NUM_INPUTDEVS; i++)
		if (inputdevs_fd[i] > -1 && FD_ISSET(inputdevs_fd[i], &set))
			break;
	return i;
}
