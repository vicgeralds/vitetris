#include <string.h>
#include <stdio.h>
#include "menu.h"
#include "menuext.h"
#include "../input/input.h"
#include "../textgfx/textgfx.h"
#include "../netw/sock.h"
#include "../options.h"

extern FILE *inet_out;

static char host_str[75];
static char name_str[18];

static int cursor = -1;

static init_field(char *str, const char *val, int maxlen)
{
	memset(str, ' ', maxlen+1);
	if (val) {
		strncpy(str, val, maxlen);
		while (!str[--maxlen])
			str[maxlen] = ' ';
	}
}

static void printtextfield(char *s, int n)
{
	int c = 0;
	int d = cursor - 31;
	if (d > 0) {
		s += d;
		n -= d;
	} else
		d = 0;
	if (n > 32) {
		c = s[32];
		s[32] = '\0';
	}
	if (cursor == -1) {
		s[n] = '\0';
		printstr(s);
		s[n] = ' ';
		if (c)
			printstr("...");
	} else
		printtextbox(s, cursor - d);
	if (c)
		s[32] = c;
	cleartoeol();
}

static int get_field_len(const char *s, int n)
{
	while (n > 0 && s[n-1] == ' ')
		n--;
	return n;
}

static int textfield(int k, int *pos)
{
	char *s;
	int n, max;
	if (*pos == 0) {
		s = host_str;
		if (k == MVDOWN)
			cursor = -1;
	} else {
		s = name_str;
		if (k == MVUP)
			cursor = -1;
	}
	max = strlen(s);
	n = get_field_len(s, max);
	if (!k || k == '\t')
		cursor = -1;
	if (cursor == -1) {
		if (k == MVRIGHT) {
			if (!n)
				k = MVLEFT;
			else
				cursor = n-1;
		}
		if (k == MVLEFT || k == DEL)
			cursor = 0;
		if (isprintable(k)) {
			memset(s, ' ', max);
			cursor = 0;
		}
	}
	if (cursor == -1) {
		printtextfield(s, n);
		return 0;
	}
	if (!isprintable(k))
		switch (k) {
		case MVLEFT:
			if (cursor > 0)
				cursor--;
			break;
		case MVRIGHT:
			if (cursor < max-1)
				cursor++;
			break;
		case BACKSPACE:
			if (!cursor)
				break;
			cursor--;
		case DEL:
			memmove(s+cursor, s+cursor+1, max-cursor-1);
			break;
		default:
			return 0;
		}
	else if (cursor < max-1 && s[max-2] == ' ') {
		memmove(s+cursor+1, s+cursor, max-cursor-2);
		s[cursor] = k;
		cursor++;
	}
	printtextfield(s, max);
	return 1;
}

static void printline(char *line)
{
	int n = strlen(line);
	if (line[n-1] == '\n')
		line[n-1] = '\0';
	printstr(line);
}

static void read_print_message(FILE *fp, int x, int y)
{
	char line[80];
	if (fp) {
		fseek(fp, 0, SEEK_SET);
		if (fgets(line, 80, fp)) {
			setcurs(x, y);
			printline(line);
			cleartoeol();
		}
		if (fgets(line, 80, fp)) {
			newln(x);
			printline(line);
		}
		fclose(fp);
	}
}

static int try_connect(int x, int y)
{
	char host[74];
	char *s;
	int n;
	clearbox(x, y, 0, 2);
	setcurs(x, y);
	n = get_field_len(host_str, 73);
	if (!n)
		return 0;
	s = host_str;
	while (*s == ' ') {
	       s++;
	       n--;
	}
	memcpy(host, s, n);
	host[n] = '\0';
	s = strrchr(host, ':');
	if (s) {
		*s = '\0';
		if (!s[1])
			s = NULL;
	}
	if (!s)
		n = 34034;
	else {
		do s++;
		while (*s == ' ');
		n = -1;
		if (isdigit(*s))
			n = atoi(s);
		if (n > 0xFFFF || n <= 0) {
			printstr("Invalid port number");
			return 0;
		}
	}
	inet_out = tmpfile();
	printstr("Connecting...");
	newln(x);
	refreshwin(-1);
	n = mksocket_inet(host, n);
	read_print_message(inet_out, x, y);
	inet_out = NULL;
	return n;
}

static void save_connect_str(const char *key, char *str)
{
	union val v;
	int n;
	v.p = str;
	n = get_field_len(str, strlen(str));
	str[n] = '\0';
	setoption("", key, v, 2);
	str[n] = ' ';
}

int netplay_menu(int x, int y)
{
	const char *menu[2] = {"Host", "Your name"};
	init_field(host_str, getopt_str("", "host"), 73);
	init_field(name_str, getopt_str("", "name"), 16);
	if (host_str[0] == ' ')
		memcpy(host_str, "localhost:34034", 15);
	setcurs(x, y);
	printstr("Connect:");
	menuhandler handlers[2] = {textfield, textfield};
	edit_mode = 1;
	while (openmenu(menu, 2, 0, x, y+2, handlers)) {
		drawmenu(menu, 2, -1, x, y+2, NULL);
		if (try_connect(x, y+5)) {
			save_connect_str("host", host_str);
			save_connect_str("name", name_str);
			edit_mode = 0;
			return 1;
		}
		newln(x);
		rmsocket();
		if (host_str[0] == ' ')
			break;
	}
	edit_mode = 0;
	clearbox(x, y, 0, 7);
	return 0;
}
