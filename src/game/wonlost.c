#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include "game.h"
#include "../options.h"
#include "../cfgfile.h"

static char *get_wonlost_stats_key(const char *me, const char *opp, char **key)
{
	struct option *o = getoptions("wonlost");
	char *s, *p;
	int n = strlen(opp);
	int m = strlen(me);
	for (; o; o = o->next) {
		s = opt_longstr(o);
		if (!s)
			continue;
		p = strrchr(s, ' ');
		if (m && !strncmp(s, me, m) && s[m] == ' ' && p > s+m)
			s += m + 1;
		if (p && isdigit(p[1]) && n == p-s &&
		    !strncmp(s, opp, n) && strlen(p) <= 8) {
			if (key)
				*key = opt_key(o);
			return p+1;
		}
	}
	return NULL;
}

const char *get_wonlost_stats(const char *me, const char *opponent)
{
	char *s = get_wonlost_stats_key(me, opponent, NULL);
	if (s)
		return s;
	return "0-0";
}

static int key2index(const char *key)
{
	int i;
	if (key[2] || !islower(key[0]))
		return -1;
	i = key[0] - 'a';
	if (key[1]) {
		if (!islower(key[1]))
			return -1;
		i *= 26;
		i += key[1] - 'a';
	}
	return i;
}

static int get_next_index(int next_index, struct option *first,
					  struct option *end)
{
	struct option *o = first;
	for (; o != end; o = o->next)
		if (key2index(opt_key(o)) == next_index)
			next_index = get_next_index(next_index+1, first, o);
	return next_index;
}

static int get_any_index(struct option *first, const char *me)
{
	struct option *o = first;
	const char *s;
	int n = strlen(me);
	for (; o != NULL; o = o->next) {
		s = opt_longstr(o);
		if (!s || strncmp(s, me, n))
			return key2index(opt_key(o));
	}
	return get_any_index(first, "");
}

static void setopt_wonlost(const char *key,
			   const char *me,
			   const char *opp,
			   const char *stats)
{
	union val val;
	char buf[44];
	sprintf(buf, "%s %s %s", me, opp, stats);
	val.p = buf;
	setoption("wonlost", key, val, 2);
}

static void add_wonlost_stats(const char *me, const char *opp, int won)
{
	struct option *first = getoptions("wonlost");
	char key[4] = "";
	int i = get_next_index(0, first, NULL);
	if (i >= 640)
		i = get_any_index(first, me);
	key[0] = 'a' + i / 26;
	key[1] = 'a' + i % 26;
	setopt_wonlost(key, me, opp, won ? "1-0" : "0-1");
}

void upd_wonlost_stats(const char *me, const char *opponent, int won)
{
	char stats[8];
	char *key;
	union val val;
	char *s, *p;
	int loss = 0;
	readoptions();
	s = get_wonlost_stats_key(me, opponent, &key);
	if (!s)
		add_wonlost_stats(me, opponent, won);
	else {
		if (won)
			won = 1;
		else
			loss = 1;
		p = strchr(s, '-');
		if (p && isdigit(p[1])) {
			won  += atoi(s);
			loss += atoi(p+1);
			if (won >= 1000)
				won = 999;
			if (loss >= 1000)
				loss = 999;
		}
		sprintf(stats, "%d-%d", won, loss);
		if (strlen(stats) <= strlen(s))
			strcpy(s, stats);
		else
			setopt_wonlost(key, me, opponent, stats);
	}
	writeconfig();
}
