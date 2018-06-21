#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "escseq.h"
#include "input.h"
#include "termin.h"

static struct escseq {
	char str[3];
	unsigned char keypress;
	struct escseq *next;
} escseq_singlepl;

#ifdef TWOPLAYER
static struct escseq escseq_1p, escseq_2p;
#endif

/* wait 1 millisecond for input */
static int readchr_wait()
{
	if (waitinput_stdin(1))
		return readchr();
	return 0;
}

int readescseq(char *s)
{
	/* wait 55 ms before returning ESC */
	int c = waitinput_stdin(55) ? readchr() : 0;
	if (c == '[') {
		c = readchr_wait();
		if (!c)
			return -1;
		switch (c) {
		case 'A': return MVUP;
		case 'B': return MVDOWN;
		case 'C': return MVRIGHT;
		case 'D': return MVLEFT;
		case '3':
			if ((c = readchr()) == '~')
				return DEL;
			if (c)
				unreadchr = c;
			c = '3';
		}
		s[0] = '[';
		s[1] = c;
		s[2] = readchr_wait();
		if (isdigit(c) && isdigit(s[2]))
			readchr_wait();
	} else {
		if (!c)
			return ESC;
		if (c == ESC) {
			unreadchr = ESC;
			return ESC;
		}
		s[0] = c;
		if (s[1] = readchr_wait())
			s[2] = readchr_wait();
	}
	s[3] = '\0';
	return 0;
}

static struct escseq *getescseq(int flags)
{
#ifdef TWOPLAYER
	if (flags & PLAYER_1)
		return &escseq_1p;
	else if (flags & PLAYER_2)
		return &escseq_2p;
	else
#endif
	return &escseq_singlepl;
}

int transl_escseq(const char *s, int flags)
{
	struct escseq *e;
	int ret;
#ifdef TWOPLAYER
	if (!(flags & SINGLE_PL)) {
		if (ret = transl_escseq(s, flags | PLAYER_1 | SINGLE_PL))
			return ret | PLAYER_1;
		if (ret = transl_escseq(s, flags | PLAYER_2 | SINGLE_PL))
			return ret | PLAYER_2;
		return 0;
	}
#endif
	e = getescseq(flags);
	ret = 0;
	do if ((flags & IN_GAME || !(e->keypress & 0x80)) &&
					!strncmp(s, e->str, 3)) {
		ret = e->keypress;
		if (ret & 0x80)
			ret ^= 0x80;
		else if (flags & IN_GAME)
			continue;
		break;
	} while (e = e->next);
	return ret;
}

const char *getescseq_str(int keypr)
{
	struct escseq *e = getescseq(keypr);
	unsigned char k = keypr | (keypr & IN_GAME) >> 1;
	do if (k == e->keypress)
		return e->str;
	while (e = e->next);
	return "";
}

void mapescseq(const char *str, int keypr)
{
	struct escseq *e = getescseq(keypr);
	unsigned char k = keypr | (keypr & IN_GAME) >> 1;
	if (e->keypress) {
		while (k != e->keypress) {
			if (!e->next) {
				e = e->next = malloc(sizeof(struct escseq));
				e->next = NULL;
				break;
			}
			e = e->next;
		}
	}
	memcpy(e->str, str, 3);
	e->keypress = k;
}

void rmescseq(int keypress)
{
	struct escseq *e, *p;
	unsigned char keypr = keypress | (keypress & IN_GAME) >> 1;
	e = getescseq(keypress);
	if (keypr == e->keypress) {
		e->keypress = 0;
		return;
	}
	while (e->next) {
		if (keypr == e->next->keypress) {
			p = e->next->next;
			free(e->next);
			e->next = p;
			break;
		}
		e = e->next;
	}
}

static void freeescseq(struct escseq *hd)
{
	struct escseq *e;
	while (e = hd->next) {
		hd->next = e->next;
		free(e);
	}
}

void free_escape_sequences()
{
	freeescseq(&escseq_singlepl);
#ifdef TWOPLAYER
	freeescseq(&escseq_1p);
	freeescseq(&escseq_2p);
#endif
}
