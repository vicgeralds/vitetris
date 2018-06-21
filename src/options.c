#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "options.h"

struct sect sect_hd = {""};

int strtoval(char *str, union val *val)
{
	int n, i;
	while (isspace(*str))
		str++;
	n = strlen(str);
	while (n && isspace(str[n-1]))
		n--;
	val->integ = 0;
	if (n==1 && *str=='0')
		return 0;
	if (n >= 4) {
		str[n] = '\0';
		val->p = str;
		return 2;
	}
	i = *str=='-';
	if (str[i]>'0' && str[i]<='9') {
		val->integ = atoi(str);
		i++;
		for (; i<n; i++)
			if (!isdigit(str[i]))
				goto str;
		return 0;
	}
str:	strncpy(val->str, str, 4);
	if (n && n<4)
		val->str[n] = '\0';
	return 1;
}

static struct sect *getsect(const char *name, struct sect **prev)
{
	struct sect *s = &sect_hd;
	if (!name[0])
		return s;
	while (s->next) {
		if (!strcmp(name, s->next->name)) {
			if (prev)
				*prev = s;
			return s->next;
		}
		s = s->next;
	}
	return NULL;
}

struct sect *addsect(const char *name)
{
	struct sect *s = getsect(name, NULL);
	if (!s) {
		s = malloc(sizeof(struct sect));
		strcpy(s->name, name);
		s->opts = NULL;
		s->next = sect_hd.next;
		sect_hd.next = s;
	}
	return s;
}

static struct option *newopt(const char *key, union val val, int tp)
{
	struct option *o;
	int n = sizeof(struct option) + strlen(key) - 2;
	int m = 0;
	char *p;
	if (tp == 1 && val.str[3])
		m = 5;
	if (tp == 2)
		m = strlen(val.p)+1;
	o = malloc(n+m);
	o->val = val;
	if (m) {
		p = (char *) o;
		p += n;
		if (tp == 2) {
			strncpy(o->val.str, val.p, 4);
			strcpy(p, val.p);
		} else {
			memcpy(p, val.str, 4);
			p[4] = '\0';
		}
		tp = 1;
	}
	o->tp_key[0] = tp;
	strcpy(opt_key(o), key);
	return o;
}

void addopt(const char *key, union val val, int tp, struct sect *sect)
{
	struct option *o = newopt(key, val, tp);
	if (!sect)
		sect = &sect_hd;
	o->next = sect->opts;
	sect->opts = o;
}

struct option *getoptions(const char *sect_name)
{
	struct sect *s = getsect(sect_name, NULL);
	if (s)
		return s->opts;
	return NULL;
}

/* getopt might be declared in stdio.h */
static struct option *get_opt(const char *sect_name, const char *key)
{
	struct option *o = getoptions(sect_name);
	while (o) {
		if (!strcmp(key, opt_key(o)))
			return o;
		o = o->next;
	}
	return NULL;
}

int getopt_int(const char *sect_name, const char *key)
{
	struct option *o = get_opt(sect_name, key);
	if (o)
		return o->val.integ;
	return 0;
}

char *getopt_str(const char *sect_name, const char *key)
{
	struct option *o = get_opt(sect_name, key);
	if (o)
		return opt_longstr(o);
	return NULL;
}

char *opt_longstr(struct option *o)
{
	if (opt_isint(o))
		return NULL;
	if (!o->val.str[3])
		return o->val.str;
	return (char *) o + (sizeof(struct option) + strlen(opt_key(o)) - 2);
}

static void rmopt(const char *key, struct option *o)
{
	struct option *p;
	while (o->next) {
		if (!strcmp(key, opt_key(o->next))) {
			p = o->next;
			o->next = p->next;
			free(p);
		} else
			o = o->next;
	}
}

void setoption(const char *sect_name, const char *key, union val val, int tp)
{
	struct sect *s = addsect(sect_name);
	struct option *o;
	if (tp==0 || tp==1 && !val.str[3]) {
		o = s->opts;
		while (o) {
			if (!strcmp(key, opt_key(o))) {
				o->val = val;
				o->tp_key[0] = tp;
				rmopt(key, o);
				return;
			}
			o = o->next;
		}
	}
	addopt(key, val, tp, s);
	rmopt(key, s->opts);
}

void unsetoption(const char *sect_name, const char *key)
{
	struct sect *s = getsect(sect_name, NULL);
	struct option *o;
	if (!s || !s->opts)
		return;
	o = s->opts;
	if (!strcmp(key, opt_key(o))) {
		s->opts = o->next;
		free(o);
	} else
		rmopt(key, o);
}

void freeoptions(const char *sect_name)
{
	struct sect *s, *p;
	struct option *o;
	if (sect_name[0]) {
		s = getsect(sect_name, &p);
		if (!s)
			return;
		p->next = s->next;
	} else {
		while (sect_hd.next)
			freeoptions(sect_hd.next->name);
		s = &sect_hd;
	}
	while (o = s->opts) {
		s->opts = o->next;
		free(o);
	}
	if (sect_name[0])
		free(s);
	else
		s->opts = NULL;
}
