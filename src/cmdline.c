#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "config.h"
#ifdef HAVE_CONIO_H
#include <conio.h>  /* getch */
#endif
#ifdef TTY_SOCKET
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif
#include "version.h"
#include "cfgfile.h"
#include "lang.h"
#include "options.h"
#include "hiscore.h"
#include "game/game.h"
#include "game/tetris.h"	/* MODE_BTYPE */
#include "textgfx/textgfx.h"
#include "input/input.h"
#include "menu/menu.h"		/* inputsetup_menu */
#include "input/joystick.h"	/* js_open */
#include "netw/sock.h"

struct cmdopt {
	const char *pattern;
	int (*func)(const char *name, char *arg);
};

#if ALLEGRO && WIN32 && !ALLEGRO_USE_CONSOLE
static int print_stdout_exit(int code)
{
	textgfx_init();
	init_inputdevs();
	getkeypress_block(0);
	textgfx_end();
	exit(code);
}
#else
#define print_stdout_exit(code) exit(code)
#endif

static int subexplen(const char *s)
{
	int n = 0;
	while (s[n] && s[n] != '|')
		n++;
	return n;
}

static int charmatches(int c, const char *charclass)
{
	do {
		charclass++;
		if (c == *charclass)
			return 1;
	} while (*charclass != ']');
	return 0;
}

static int namematches(const char *name, const char *pattern)
{
	int n;
	while (n = subexplen(pattern)) {
		if (*pattern =='[' && !name[1]) {
			if (charmatches(*name, pattern))
				return 1;
		}
		else if (!strncmp(name, pattern, n) && n == strlen(name))
			return 1;
		pattern += n;
		if (*pattern =='|')
			pattern++;
	}
	return 0;
}

static int argtonum(const char *arg)
{
	if (arg) {
		while (isspace(*arg))
			arg++;
		if (isdigit(*arg))
			return atoi(arg);
	}
	return -1;
}

static void x_arg_error(const char *x, const char *name, const char *try)
{
	printf("%s argument for option \"%s\".  ", x, name);
	puts(try);
	print_stdout_exit(1);
}

static void invalid_arg(const char *name, const char *try)
{
	x_arg_error("Invalid", name, try);
}

static void requires_arg(const char *name, char *arg, const char *try)
{
	if (!arg)
		x_arg_error("Missing", name, try);
}

static void set_mode_btype()
{
	union val v;
	v.integ = getopt_int("", "mode") | MODE_BTYPE;
	setoption("", "mode", v, 0);
}

static int cmd_game_opt(const char *name, char *arg)
{
	const char *key = name;
	int val;
	int pl = 0;
	const char *sect = "";
	union val v;
	val = argtonum(arg);
#ifdef TWOPLAYER
	if (*key =='p') {
		pl = key[1]-'0';
		switch (pl) {
		case 1: sect = "player1"; break;
		case 2: sect = "player2"; break;
		default:
			return 0;
		}
		key += 2;
	}
#endif
	switch (testgameopt(key, val, pl)) {
	case -1:
		if (!strcmp(name, "height"))
			return 0;
		invalid_arg(name, "Try -help game.");
		print_stdout_exit(1);
	case 0:
		return 0;
	case 1:
		if (arg[1] =='b')
			val |= MODE_BTYPE;
		break;
	case 4:
		set_mode_btype();
	}
	v.integ = val;
	setoption(sect, key, v, 0);
	return 1;
}

static int cmd_wh(const char *name, char *arg)
{
	int n;
	if ((!arg || islower(*arg)) && !strcmp(name, "h"))
		return 0;
	n = argtonum(arg);
	if (n < 0)
		invalid_arg(name, "");
	if (*name =='w')
		settermwidth(n);
	else
		settermheight(n);
	return 1;
}

static void print_opt(char *buf, int tabw, const char *a)
{
	int n = 0;
	char *p;
	p = strchr(buf, '\t');
	if (!p) {
		printf("  -");
		p = buf;
	} else {
		*p = '\0';
		p++;
		if (buf[0])
			n = printf("  -%s ", buf);
		if (n > tabw) {
			putchar('\n');
			n = 0;
		}
		do putchar(' ');
		while (++n < tabw);
	}
	printf(p, a);
	putchar('.');
}

static void printopts(int tabw, const char *a, const char *s)
{
	char buf[80];
	const char *p;
	while (*s) {
		p = strchr(s, '\n');
		if (p) {
			memcpy(buf, s, p-s);
			buf[p-s] = '\0';
			s = p+1;
		} else {
			strcpy(buf, s);
			s = "";
		}
		if (buf[0])
			print_opt(buf, tabw, a);
		putchar('\n');
	}
}

static void printhelp()
{
	printf(VITETRIS_VER" ("VERSION_DATE")"
#if WIN32
	  "  WIN32"
#elif MSDOS
	  "  MSDOS"
#endif
	  "\n\n");
	printopts(18, NULL,
#ifdef ALLEGRO
	  "fullscreen	Fullscreen mode\n"
	  "fullscreen 0	Windowed mode\n"
#endif
#ifndef NO_MENU
	  "nomenu	Skip menus\n"
#endif
	  "hiscores	Print highscore list and exit\n"
	  "hiscores FILE	Read and add highscore entries from FILE\n"
#if NO_MENU && JOYSTICK
	  "inputsetup [N]	Set keys/buttons for player N\n"
#elif NO_MENU
	  "inputsetup [N]	Set keys for player N\n"
#endif
#if JOYSTICK && !ALLEGRO
	  "js0 DEVNAME	Joystick device name (e.g. /dev/js0)\n"
	  "js1 DEVNAME	Second joystick device name\n"
#endif
	);
#ifdef SOCKET
	printopts(18, "Connect to ",
# ifdef INET
	  "listen PORT	Listen for connections on port PORT\n"
	  "connect HOSTNAME:PORT"
	 	"	%sHOSTNAME on PORT.  HOSTNAME may be an IP address\n"
	  "connect PORT	%slocalhost on PORT\n"
# endif
# ifdef TTY_SOCKET
	  "listen	Listen for local (Unix domain socket) connections\n"
	  "connect TTY	%sTTY (e.g. pts/0)\n"
# endif
	  "name NAME	Identify yourself as NAME when connecting to other players"
	);
#endif
	printopts(18, "options and exit",
	   "help	Print this help and exit\n"
	   "help game	List game %s\n"
	   "help term	List terminal %s\n"
	   "?, -h [ARG]\tSame as -help ARG (if ARG isn't a number)");
	printf("\n"
	  "Options may also be given as option=ARG, --option=ARG etc.\n\n"
	  "Highscores are saved in %s"
#ifdef UNIX
	  ",\nand in "HISCORE_FILENAME" if it exists and is writable"
#endif
	  ".\n", cfgfilename);
}

static void printhelp_game()
{
	puts("Game options:");
	printopts(15, "Single-player game",
	  "mode 1	%s\n"
	  "mode 1b	%s in B-type mode\n"
	  "level 0-9	Starting level\n"
	  "height 0-5	Height of garbage blocks at beginning\n"
	  "lines N	Lines limit (B-type mode)\n"
	  "rotate 0-3	Rotation system\n"
	  "softdrop N	Softdrop speed (drop N rows)\n"
#ifdef TWOPLAYER
	  "mode 2[b]	Two-player game [B-type mode]\n"
	  "	Player options are prefixed with p1 or p2 (e.g. -p1level 9)"
#endif
	);
}

static void printhelp_term()
{
	char colors[8] = "colours";
	char alt[12] = "alternative";
	spellword(colors);
	spellword(alt);
	puts("Terminal options:");
	printopts(14, colors,
	  "w, -width N   Set width to N columns (N >= 32)\n"
	  "h, -height N  Set height to N lines (N >= 20)\n"
	  "bg black	Adjust to dark background\n"
	  "bg white	Adjust to light background\n"
	  "color	Use %s\n"
	   "mono	Don't use %s");
	printopts(14, alt,
	  "ascii	Use ASCII characters for line drawing\n"
	  "vt100	Use %s character set for line drawing\n"
#ifndef NO_BLOCKSTYLES
	  "block STR	Draw blocks using characters in STR\n"
	  "tt	Draw blocks as in Mike Taylor's Tetris for Terminals\n"
	  "tt-bg	tt block characters with background"
#endif
	);
}

static int cmd_help(const char *name, char *arg)
{
	if (arg) {
		if (!strcmp(arg, "game")) {
			printhelp_game();
			print_stdout_exit(0);
		}
		if (!strcmp(arg, "term")) {
			printhelp_term();
			print_stdout_exit(0);
		}
		if (*name =='h' && !name[1] && argtonum(arg) > -1)
			return 0;
	}
	printhelp();
	print_stdout_exit(0);
}

#ifdef ALLEGRO
static int cmd_fullscreen(const char *name, char *arg)
{
	union val v;
	v.integ = argtonum(arg) != 0;
	setoption("", "fullscreen", v, 0);
	return 1;
}
#endif

static int cmd_nomenu(const char *name, char *arg)
{
#ifndef NO_MENU
	in_menu = !argtonum(arg);
#endif
	return 1;
}

static void print_hiscorelist()
{
	char buf[320];
	if (!readhiscores(NULL))
		puts("No saved scores");
	else {
		printf("    Name      Score  Lvl  Lines\n");
		gethiscorelist(buf);
		printf(buf);
	}
}

static int cmd_hiscore(const char *name, char *fname)
{
	if (!fname) {
		print_hiscorelist();
		print_stdout_exit(0);
	}
	readhiscores(fname);
	return 1;
}

#ifdef NO_MENU
static int cmd_inputsetup(const char *name, char *arg)
{
	const char *plstr = "single player";
	int pl = argtonum(arg);
	if (pl < 0 || pl > 2)
		pl = 0;
	gettermoptions();
	textgfx_init();
	atexit(textgfx_end);
	init_inputdevs();
#ifdef TWOPLAYER
	switch (pl) {
	case 1:  plstr = "player1";  break;
	case 2:  plstr = "player2";
	}
#endif
	printstr(plstr);
	newln(0);
	newln(0);
	inputsetup_menu(pl, 2, 2);
	textgfx_end();
	writeconfig_message();
	exit(0);
}
#endif /* NO_MENU */

#if JOYSTICK && !ALLEGRO
static int cmd_jsopen(const char *name, char *arg)
{
	requires_arg(name, arg, "");
	js_open(name[2]-'0', arg);
	return 1;
}
#endif

#ifdef SOCKET
static int test_port_number(int n)
{
	if (n > 0xFFFF) {
		puts("Port number can't be greater than 65535.");
		print_stdout_exit(1);
	}
	return n > 0;
}

static void press_any_key()
{
#ifdef HAVE_CONIO_H
	printf("Press any key to continue ");
	getch();
	putchar('\n');
#else
	printf("Press ENTER to continue ");
	getchar();
#endif
}

static int cmd_listen(const char *name, char *arg)
{
	char *msg;
	unsigned port = 34034;
	int n;
#ifndef INET
	if (arg) {
		printf("Option \"%s\" doesn't allow an argument.  ", name);
		puts("Try -help for more information.");
		print_stdout_exit(1);
	}
#endif
#ifdef TTY_SOCKET
	if (!arg) {
		if (msg = mksocket_local(NULL, 1)) {
			puts(msg);
			free(msg);
			print_stdout_exit(1);
		}
		printf("Listening for connections on %s...\n"
		    "Connect from another terminal with \"-connect %s\".\n",
			get_socket_fname(), this_tty);
		goto press;
	}
#endif
#ifdef INET
	n = argtonum(arg);
	if (test_port_number(n))
		port = n;
	if (!mksocket_inet(NULL, port))
		print_stdout_exit(1);
#endif
#if !CURSES && !ALLEGRO
	return 1;
#endif
press:	press_any_key();
	return 1;
}

static char *getopt_host()
{
#ifdef INET
	static char host[84];
	const char *s = getopt_str("", "host");
	char *p;
	if (s) {
		strcpy(host, s);
		p = strchr(host, ':');
		if (!p)
			strcat(host, ":34034");
		return host;
	}
#endif
	return NULL;
}

#ifdef INET
static void save_host(const char *hostname, int port)
{
	char s[74];
	union val v;
	if (strlen(hostname) < 68) {
		sprintf(s, "%s:%d", hostname, port);
		v.p = s;
		setoption("", "host", v, 2);
	}
}
#endif

static int connect_to(char *arg, char *p)
{
#ifndef INET
	return 0;
#else
	int n = argtonum(p);
	if (!test_port_number(n))
		return 0;
	if (!*arg)
		arg = "localhost";
	if (mksocket_inet(arg, n))
		save_host(arg, n);
	else
		print_stdout_exit(1);
	return 1;
#endif
}

#ifdef TTY_SOCKET
static int is_tty_maybe(const char *tty)
{
#ifndef INET
	return 1;
#else
	char fname[16] = "/dev/";
	struct stat st;
	if (tty[0] == '/')
		return 1;
	if (strlen(tty) > 10)
		return 0;
	strcat(fname, tty);
	return stat(fname, &st) == 0;
#endif
}
#endif

static int cmd_connect(const char *name, char *arg)
{
	char *p;
	if (!arg)
		arg = getopt_host();
	requires_arg(name, arg, "Try -help for more information.");
	p = strrchr(arg, ':');
	if (p) {
		*p = '\0';
		p++;
	} else if (argtonum(arg) < 0) {
#ifdef TTY_SOCKET
		if (is_tty_maybe(arg)) {
			connect_tty(arg);
			return 1;
		}
#endif
		;
	}
#ifdef INET
	else if (!strchr(arg, '.')) {
		p = arg;
		arg = "";
	}
	if (!connect_to(arg, p)) {
		printf("You need to specify a valid port number (%s:PORT).\n"
			"34034 is a good choice.\n", arg);
		print_stdout_exit(1);
	}
#else
	invalid_arg("connect", "Expected tty.");
#endif
	return 1;
}

static int cmd_name(const char *name, char *arg)
{
	union val v;
	requires_arg(name, arg, "");
	v.p = arg;
	setoption("", "name", v, 2);
	return 1;
}
#endif /* SOCKET */

static int cmd_bg(const char *name, char *arg)
{
	union val v;
	requires_arg(name, arg, "(black/white)");
	v.integ = argtonum(arg);
	v.integ = v.integ==1 || *arg=='w';
	setoption("term", "bg", v, 0);
	return 1;
}

static int cmd_color(const char *name, char *arg)
{
	union val v;
	v.integ = argtonum(arg) != 0;
	if (*name =='m')
		v.integ = !v.integ;
	setoption("term", "color", v, 0);
	return 1;
}

static int cmd_ascii(const char *name, char *arg)
{
	union val v;
	v.integ = argtonum(arg) != 0;
	if (*name =='v')
		v.integ = !v.integ;
	setoption("term", "drawing", v, 0);
	return 1;
}

#ifndef NO_BLOCKSTYLES
static void unsetbgdot()
{
	if (!getopt_int("term", "bgdot"))
		unsetoption("term", "bgdot");
}

static int cmd_block_bgdot(const char *name, char *arg)
{
	union val v;
	requires_arg(name, arg, "");
	unsetbgdot();
	strncpy(v.str, arg, 4);
	if (v.str[0]==' ')
		v.str[0] = ' '-1;
	else if (!v.str[2])
		v.str[2] = '_';
	if (!strcmp(arg, "0"))
		v.str[1] = ' ';
	setoption("term", name, v, 1);
	return 1;
}

static int cmd_tt(const char *name, char *arg)
{
	union val v;
	if (name[2] || argtonum(arg) == 2) {
		unsetbgdot();
		v.integ = -2;
	} else {
		v.integ = 0;
		setoption("term", "bgdot", v, 1);
		v.integ = -1;
	}
	setoption("term", "block", v, 0);
	return 1;
}
#endif

static void proc_cmdline_opt(const char *name, char *arg)
{
	const struct cmdopt opts[] = {
		{"[wh]|width|height", cmd_wh},
		{"[?h]|help", cmd_help},
#ifdef ALLEGRO
		{"fullscreen", cmd_fullscreen},
#endif
		{"nomenu", cmd_nomenu},
		{"hiscores|hiscore", cmd_hiscore},
#ifdef NO_MENU
		{"inputsetup", cmd_inputsetup},
#endif
#if JOYSTICK && !ALLEGRO
		{"js0|js1", cmd_jsopen},
#endif
#ifdef SOCKET
		{"listen", cmd_listen},
		{"connect", cmd_connect},
		{"name", cmd_name},
#endif
		{"bg", cmd_bg},
		{"color|mono", cmd_color},
		{"ascii|vt100", cmd_ascii},
#ifndef NO_BLOCKSTYLES
		{"block|bgdot", cmd_block_bgdot},
		{"tt|tt-bg", cmd_tt},
#endif
		{NULL}
	};
	const struct cmdopt *o = opts;
	if (cmd_game_opt(name, arg))
		return;
	while (o->pattern) {
		if (namematches(name, o->pattern) && o->func(name, arg))
			return;
		o++;
	}
	printf("Unrecognized option \"%s\".  ", name);
	puts("Try -help for more information.");
	print_stdout_exit(1);
}

void proc_args(char **args, int n)
{
	int i, c;
	char *p, *q;
	for (i = 0; i < n; i++) {
		p = args[i];
		if (*p == '/')
			p++;
		else
			while (*p == '-')
				p++;
		if (!strlen(p))
			continue;
		if (q = strchr(p, '=')) {
			*q = '\0';
			q++;
		} else if (i+1 < n) {
			c = args[i+1][0];
			if (c != '-' && (p > args[i] || !islower(c) ||
			    namematches(p, "[?h]|help|connect")))
				q = args[i+1];
#ifdef SOCKET
			if (q && i+2 < n && !strcmp(p, "connect") &&
			    connect_to(q, args[i+2])) {
				i += 2;
				continue;
			}
#endif
		}
		proc_cmdline_opt(p, q);
		if (q && i+1 < n && q == args[i+1])
			i++;
	}
}
