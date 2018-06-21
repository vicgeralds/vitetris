#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "config.h"
#ifdef UNIX
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#endif
#include "cfgfile.h"
#include "options.h"
#include "hiscore.h"
#include "input/input.h"
#include "input/keyboard.h"
#include "input/joystick.h"
#include "draw/draw.h"

char cfgfilename[80] = CONFIG_FILENAME;

static const char input_keynames[12][6] = {
	"left", "right", "up", "down", "a", "b", "start",
	"rot", "sdrop", "rot-a", "rot-b", "hdrop"
};
static const char tcolor_keys[8] = "ijlostz";

void writehiscores(FILE *fp);

void setcfgfilename(const char *argv0)
{
#ifdef UNIX
	struct passwd *pwd;
	const char *s = getenv("HOME");
	if (!s && (pwd = getpwuid(getuid())))
		s = pwd->pw_dir;
	if (s && strlen(s)+strlen(CONFIG_FILENAME) < 79) {
		strcpy(cfgfilename, s); 
		strcat(cfgfilename, "/"CONFIG_FILENAME);
	}
#else
	int n = strlen(argv0);
	do n--;
	while (n>=0 && argv0[n] !='\\' && argv0[n] !='/');
	if (n>=0 && n+strlen(CONFIG_FILENAME) < 79) {
		strncpy(cfgfilename, argv0, n+1);
		cfgfilename[n+1] = '\0';
		strcat(cfgfilename, CONFIG_FILENAME);
	}
#endif
}

static int readname(const char *line, char *dest)
{
	int i, n;
	if (line[0] == '[') {
		line++;
		n = 7;
	} else
		n = 11;
	i = 0;
	while (i < n && (i && line[i] == '-' || islower(line[i]) ||
						isdigit(line[i]))) {
		dest[i] = line[i];
		i++;
	}
	if (!i || line[i-1] == '-')
		return 0;
	dest[i] = '\0';
	if (n == 7) {
		if (line[i] != ']')
			return 0;
	} else {
		while (isspace(line[i]))
			i++;
		if (line[i] != '=')
			return 0;
	}
	return 1;
}

static int readopt(const char *line, char *key, union val *val, int *tp)
{
	char *p;
	if (line[0] == '\n')
		return 0;
	if (readname(line, key)) {
		if (line[0] == '[')
			return 1;
		if (p = strchr(line, '=')) {
			*tp = strtoval(p+1, val);
			return 1;
		}
	}
	return 0;
}

#if JOYSTICK && TWOPLAYER
static void read_inputdev_player(int pl)
{
	int i = 0;
	if (pl != 1 && pl != 2)
		return;
	if (sect_hd.next->name[0] == 'j')
		i = sect_hd.next->name[2]-'0'+1;
	inputdevs_player[i] = pl;
}
#endif

static int nametokeypress_i(int i, int flags)
{
	if (i < 6)
		return MVLEFT+i | flags;
#ifdef JOYSTICK
	if (i == 6)
		return STARTBTN;
#endif
	if (i == 12)
		return 0;
	return (i < 11 ? MVUP+i-7 : HARDDROP) | IN_GAME | flags;
}

static int nametokeypress(char *name)
{
	int i = 0;
	int flags;
#ifdef TWOPLAYER
	if (name[0] == 'p') {
		flags = (name[1]=='1') ? PLAYER_1 : PLAYER_2;
		memmove(name, name+2, strlen(name+2)+1);
	} else
#endif
	flags = 0;
	while (strcmp(name, input_keynames[i]) && i < 12)
		i++;
	return nametokeypress_i(i, flags);
}

static int test_input_chr9(const char *s)
{
	int i = 0;
	while (i < 9) {
		if (!strcmp(s, input_chr9[i]))
			return i+1;
		i++;
	}
	return 0;
}

static unsigned char *stdin_convertval(union val *val, int tp)
{
	unsigned char bs[2] = "";
	if (!tp) {
		if (val->integ < 0)
			;
		else if (val->integ < 10)
			bs[0] = val->integ+'0';
		else if (val->integ > ' ') {
#ifdef CURSES
			bs[0] = val->integ & 0xFF;
			bs[1] = val->integ >> 8;
#else
			return NULL;
#endif
		} else
			bs[0] = val->integ;
	} else {
		if (tp == 2)
			strncpy(val->str, val->p, 4);
		if (!val->str[1])
			return (unsigned char *) val->str;
#if !CURSES
		if (val->str[0] == ESC)
			return (unsigned char *) val->str;
		else if (!val->str[3] && sscanf(val->str, "0%X", &tp) == 1)
			bs[1] = tp;
#endif
		else {
			bs[0] = test_input_chr9(val->str);
			if (!bs[0])
				return NULL;
		}
	}
	memcpy(val->str, bs, 2);
	return (unsigned char *) val->str;
}

static int readkeymapping(char *key, union val val, int tp)
{
	int keypress = nametokeypress(key);
	unsigned char *input;
	if (!keypress)
		return 0;
#ifdef JOYSTICK
	if (sect_hd.next->name[0] == 'j') {
		if (!tp)
			val.integ += '0';
		else if (tp != 1 || !(val.integ = test_input_chr9(val.str)))
			return 0;
		keypress &= 63 | IN_GAME;
		js_setmapping(sect_hd.next->name[2]=='1', val.integ, keypress);
	} else
#endif
      	if ((keypress & 63) == STARTBTN)
		;
	else if (input = stdin_convertval(&val, tp)) {
		if (input[0] | input[1])
			kb_setmapping(input, keypress);
	} else
		return 0;
	return 1;
}

static void read_tetrom_color(const char *key, int val)
{
	char *p;
	if (strlen(key) > 1 || val < 1 || val > 7)
		return;
	if (p = strchr(tcolor_keys, key[0]))
		tetrom_colors[p-tcolor_keys] = val;
}

static int is_inputconf(const char *key)
{
	return !strcmp(key, "stdin")
#ifdef JOYSTICK
		|| !strcmp(key, "js0") || !strcmp(key, "js1")
#endif
		;
}

void readoptions()
{
	FILE *fp;
	char line[80];
	char key[12];
	union val val;
	int tp;
	char inputconf = 0;
	char tcolors = 0;

	if (sect_hd.opts || sect_hd.next)
		return;
	fp = fopen(cfgfilename, "r");
	if (!fp)
		return;
	while (fgets(line, 80, fp)) {
		if (!readopt(line, key, &val, &tp))
			continue;
		if (line[0] == '[') {
			if (!strcmp(key, "hiscore"))
				break;
			inputconf = is_inputconf(key);
			tcolors = !strcmp(key, "tcolors");
			if (!tcolors)
				addsect(key);
			continue;
		}
		if (inputconf) {
#if JOYSTICK && TWOPLAYER
			if (!tp && !strcmp(key, "player"))
				read_inputdev_player(val.integ);
			else
#endif
			if (!readkeymapping(key, val, tp))
				goto addopt;
			continue;
		} else if (tcolors) {
			read_tetrom_color(key, val.integ);
			continue;
		}
addopt:		addopt(key, val, tp, sect_hd.next);
	}
	fclose(fp);
}

static void writeopts(FILE *fp, const struct sect *sect)
{
	struct option *o = sect->opts;
	while (o) {
		fprintf(fp, "%s=", opt_key(o));
		if (opt_isint(o))
			fprintf(fp, "%d", o->val.integ);
		else if (o->val.str[3])
			fputs(opt_longstr(o), fp);
		else
			fputs(o->val.str, fp);
		putc('\n', fp);
		o = o->next;
	}
}

#ifdef JOYSTICK
static void printbtnmapping(FILE *fp, int n, int keypress, int i)
{
	int b = js_getbtnfor(n, keypress);
	if (!b || b == keypress)
		return;
	keypress ^= IN_GAME;
	if (b == keypress && b == js_getbtnfor(n, keypress))
		return;
	fprintf(fp, "%s=", input_keynames[i]);
	if (b < 10)
		fprintf(fp, input_chr9[b-1]);
	else
		fprintf(fp, "%d", b-'0');
	putc('\n', fp);
}
#else
#define printbtnmapping(fp, n, k, i)
#endif

static void printkeymapping(FILE *fp, int keypress, int i)
{
	unsigned char s[5] = "";
	int n = kb_getkeyfor(keypress, s, 0);
	if (!n)
		return;
#ifdef TWOPLAYER
	if (keypress & (PLAYER_1 | PLAYER_2))
		fprintf(fp, "p%c", '1'+!(keypress & PLAYER_1));
#endif
	fprintf(fp, "%s=", input_keynames[i]);
	if (n > 1) {
#if CURSES
		n  = s[0];
		n |= s[1] << 8;
		fprintf(fp, "%d", n);
#else
		if (s[0])
			fputs(s, fp);
		else
			fprintf(fp, "0%X", s[1]);
#endif
		putc('\n', fp);
		return;
	}
	if (s[0] > ' ')
		putc(s[0], fp);
	else if (s[0] < 10)
		fprintf(fp, input_chr9[s[0]-1]);
	else
		fprintf(fp, "%d", s[0]);
	putc('\n', fp);
}

#if JOYSTICK
#define isjoystick (devkey[0]=='j')
#else
#define isjoystick 0
#endif

static void writeinputconf(FILE *fp, const char *devkey)
{
	int keypress, flags = 0;
	int i = 0;
	if (isjoystick)
		i = devkey[2]-'0'+1;
#if JOYSTICK && TWOPLAYER
	if (inputdevs_player[i])
		fprintf(fp, "player=%d\n", inputdevs_player[i]);
#endif

mappings:
	i = -1;
	do {
		if (i == 5 && !isjoystick || i == 7)
			i += 2;
		else if (i == 11)
			i = 8;
		else
			i++;
		keypress = nametokeypress_i(i, flags);
		if (isjoystick)
			printbtnmapping(fp, devkey[2]=='1', keypress, i);
		else
			printkeymapping(fp, keypress, i);
	} while (i != 8);

	if (isjoystick)
		return;
#ifdef TWOPLAYER
	if (!flags) {
		flags = PLAYER_1;
		goto mappings;
	}
	if (flags & PLAYER_1) {
		flags = PLAYER_2;
		goto mappings;
	}
#endif
}

static void write_tetrom_colors(FILE *fp)
{
	int i;
	if (!strncmp(tetrom_colors, "\x1\x7\x5\x4\x2\x3\x6", 7))
		return;
	fprintf(fp, "[tcolors]\n");
	for (i = 0; i < 7; i++)
		fprintf(fp, "%c=%d\n", tcolor_keys[i], tetrom_colors[i]);
}

int writeconfig()
{
	FILE *fp;
	struct sect *sect;
	readoptions();
	readhiscores(NULL);
	fp = fopen(cfgfilename, "w");
	if (!fp)
		return 0;
	writeopts(fp, &sect_hd);
	addsect("stdin");
#if ALLEGRO
	addsect("js0");
	addsect("js1");
#endif
	sect = sect_hd.next;
	while (sect) {
		fprintf(fp, "[%s]\n", sect->name);
		if (is_inputconf(sect->name))
			writeinputconf(fp, sect->name);
		writeopts(fp, sect);
		sect = sect->next;
	}
	freeoptions("");
	write_tetrom_colors(fp);
	if (hiscores[0].score) {
		fprintf(fp, "[hiscore]\n");
		writehiscores(fp);
	}
	fclose(fp);
	return 1;
}

void writeconfig_message()
{
	if (!writeconfig())
		printf("ERROR! Could not save configuration to %s\n",
			cfgfilename);
}
