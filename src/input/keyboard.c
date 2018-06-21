#include <string.h>
#include "keyboard.h"
#include "input.h"
#include "escseq.h"

int kb_no_autorep = 0;

static struct keys {
	unsigned short menu_keys[6];
	unsigned short ingame_keys[5];
} keys_singlepl;

#ifdef TWOPLAYER
static struct keys keys_1p, keys_2p;
#endif

static int transl_chr_ingame(unsigned c, unsigned short *keys)
{
	int i = 0;
	keys += keys_singlepl.ingame_keys - keys_singlepl.menu_keys;
	while (i < 5) {
		if (keys[i] == c)
			return i==4 ? HARDDROP : MVUP+i;
		i++;
	}
	return 0;
}

static int transl_chr(unsigned c, int k)
{
	unsigned short *keys;
	int i;
#ifdef TWOPLAYER
	if (k & PLAYER_1)
		keys = keys_1p.menu_keys;
	else if (k & PLAYER_2)
		keys = keys_2p.menu_keys;
	else if (!(k & SINGLE_PL)) {
		if (i = transl_chr(c, k | PLAYER_1))
			return i | PLAYER_1;
		if (i = transl_chr(c, k | PLAYER_2))
			return i | PLAYER_2;
		return 0;
	} else
#endif
	keys = keys_singlepl.menu_keys;
	if ((k & IN_GAME) && (k = transl_chr_ingame(c, keys)))
		return k;
	for (i = 0; i < 6; i++)
		if (keys[i] == c)
			return MVLEFT+i;
	return 0;
}

static int transl_input(const unsigned char *input, int flags)
{
	unsigned c = *input;
#ifdef TWOPLAYER
	if (!(flags & (PLAYER_1 | PLAYER_2))) {
		if (inputdevs_player[0] == 1)
			flags |= PLAYER_1 | SINGLE_PL;
		if (inputdevs_player[0] == 2)
			flags |= PLAYER_2 | SINGLE_PL;
	}
#endif
#ifdef ESCSEQ
	if (c == ESC)
		return transl_escseq((const char *) input+1, flags);
#else
	c |= input[1] << 8;
#endif
	return transl_chr(c, flags);
}

int kb_getpress(int flags)
{ 
	unsigned char s[5] = "";
	unsigned c;
	if (!kb_readkey(s))
		return 0;
	if (c = transl_input(s, flags))
		return c;
	if (!s[1])
		c = s[0];
	else if (flags & IN_GAME)
		c = ESC+1;
	else
		c =  kb_toascii(s);
	if (!(flags & IN_GAME) || c <= MVDOWN
#ifdef TWOPLAYER
		&& (flags & SINGLE_PL || inputdevs_player[0] ||
		    !keys_1p.menu_keys[c-MVLEFT])
#endif
	)
		return c;
	if (strchr("\033\b \npq", c))
		return c;
	return kb_getpress(flags);
}

static unsigned short *getchrfor(int keypress)
{
	unsigned short *keys;
	int i;
#ifdef TWOPLAYER
	if (keypress & PLAYER_1)
		keys = keys_1p.menu_keys;
	else if (keypress & PLAYER_2)
		keys = keys_2p.menu_keys;
	else
#endif
	keys = keys_singlepl.menu_keys;
	if (keypress & IN_GAME) {
		keys += keys_singlepl.ingame_keys - keys_singlepl.menu_keys;
		if ((keypress & 63) == HARDDROP)
			i = 4;
		else
			i = (keypress & 7)-MVUP;
	} else
		i = (keypress & 7)-MVLEFT;
	return keys+i;
}

static int key_equals(int keypr, const unsigned char *input)
{
	unsigned c = *input;
#if ESCSEQ
	if (c == ESC)
		return !strncmp((const char *) input+1,
				getescseq_str(keypr),  3);
#else
	c |= input[1] << 8;
#endif
	return c == *getchrfor(keypr);
}

static int transl_strict(const unsigned char *input, int flags)
{
	int k = transl_input(input, flags | SINGLE_PL);
	if (flags & IN_GAME && !key_equals(k | flags, input))
		k = 0;
	return k;
}

static void rmkey(const unsigned char *input, int keypr)
{
	int flags = keypr & (IN_GAME | PLAYER_1 | PLAYER_2);
	int old = transl_strict(input, flags);
	if (old && old != (keypr & 63))
		kb_rmmapping(old | flags);
#ifdef TWOPLAYER
	if (!(flags & (PLAYER_1 | PLAYER_2)))
		return;
	flags ^= PLAYER_1 | PLAYER_2;
	if (old = transl_strict(input, flags))
		kb_rmmapping(old | flags);
	flags ^= IN_GAME;
	if (old = transl_strict(input, flags))
		kb_rmmapping(old | flags);
#endif
}

static int ingame_key_equals(int keypr, unsigned c)
{
	const char *s;
	if (keypr & IN_GAME || (keypr & 7) < MVUP)
		return 0;
#ifdef ESCSEQ
	if (!c) {
		s = getescseq_str(keypr);
		return *s && !strncmp(s, getescseq_str(keypr | IN_GAME), 3);
	}
#endif
	return c && c == *getchrfor(keypr | IN_GAME);
}

void kb_setmapping(const unsigned char *input, int keypr)
{
	unsigned short *c = getchrfor(keypr);
	rmkey(input, keypr);
	if (ingame_key_equals(keypr, *c))
		kb_setmapping(input, keypr | IN_GAME);
#ifdef ESCSEQ
	if (!input[0])
		return;
	if (input[0] == ESC) {
		*c = 0;
		mapescseq((const char *) input+1, keypr);
		return;
	}
	if (*getescseq_str(keypr))
		rmescseq(keypr);
#endif
	*c  = input[0];
#ifndef ESCSEQ
	*c |= input[1] << 8;
#endif
}

static int test_fallback(int keypr, const unsigned char *input)
{
	int flags = keypr & IN_GAME;
	int k = keypr & 63;
#ifdef TWOPLAYER
	int pl = keypr & (PLAYER_1 | PLAYER_2);
	if (!pl)
		flags |= SINGLE_PL;
#endif
	if ((!(keypr & IN_GAME) || k==HARDDROP) && (k==A_BTN || k==B_BTN
#ifdef TWOPLAYER
			|| pl==PLAYER_2 && inputdevs_player[0] != 2
#endif
				))
		return 0;
	keypr = transl_input(input, flags);
	return !keypr ||
#ifdef TWOPLAYER
		pl == (keypr & (PLAYER_1 | PLAYER_2)) &&
#endif
		k == (keypr & 63);
}

int kb_getkeyfor(int keypr, unsigned char *dest, int fallback)
{
	unsigned c = *getchrfor(keypr);
#ifdef ESCSEQ
	const char *s;
	if (c) {
		*dest = c;
		return 1;
	}
	s = getescseq_str(keypr);
	if (*s) {
		dest[0] = ESC;
		strncpy((char *) dest+1, s, 3);
		dest[4] = '\0';
		return strlen((char *) dest);
	}
#else
	if (c) {
		*dest = c & 0xFF;
		if (c >>= 8) {
			dest[1] = c;
			return 2;
		}
		return 1;
	}
#endif
	if (!fallback)
		return 0;
	*dest = keypr;
	dest[1] = 0;
	c = 1;
	if (keypr & IN_GAME && *dest != HARDDROP)
		c = kb_getkeyfor(keypr ^ IN_GAME, dest, 1);
	if (c && !test_fallback(keypr, dest))
		c = 0;
	return c;
}

int kb_getchrfor(int keypress)
{
	return *getchrfor(keypress);
}

void kb_rmmapping(int keypress)
{
	unsigned short *c = getchrfor(keypress);
	*c = 0;
#ifdef ESCSEQ
	rmescseq(keypress);
#endif
}
