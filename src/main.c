#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>

#ifdef ALLEGRO
#include <stdio.h>
#include "config.h"
#include <allegro.h>
#include "textgfx/alleg.h"
#else
#define END_OF_MAIN()
#endif

#include "timer.h"
#include "cfgfile.h"
#include "options.h"
#include "lang.h"
#include "focus.h"
#include "textgfx/textgfx.h"
#include "input/input.h"

#include "game/tetris.h"
#include "game/game.h"
#include "menu/menuext.h"
#include "netw/sock.h"

#ifdef TTY_SOCKET
#define mk_invitfile() mkinvitfile()
#define rm_invitfile() rminvitfile()
#else
#define mk_invitfile()
#define rm_invitfile()
#endif

/* startup menu items */
#ifdef INET
#define OPTIONS 6
#define HIGHSCORES 7
#else
#define OPTIONS 5
#define HIGHSCORES 6
#endif

void proc_args(char **args, int n);

static void setupgame(int i)
{
	allocgame(i & 2);
	initgame();
#if !NO_MENU
	if (!in_menu)
		goto skipmenu;
	while (gamemenu()) {
		in_menu = 0;
		rm_invitfile();
skipmenu:
#endif
		writeconfig();
		do textgfx_entergame();
		while (startgame());
#if !NO_MENU
		clearwin(0);
		in_menu = 1;
		textgfx_entermenu();
		readoptions();
	}
	inputdevs_player[0] = 0;
# ifdef SOCKET
	rmsocket();
	mk_invitfile();
# endif
#endif
	game = NULL;
}

static void startup(int i)
{
#if XLIB
	in_xterm_init();
#endif
	textgfx_init();
	atexit(textgfx_end);
	init_inputdevs();
#if TERM_RESIZING
	enable_term_resizing();
#endif
	if (!bgdot)
		bgdot = default_bgdot();
	if (in_menu)
		textgfx_entermenu();
#if !TWOPLAYER
	setupgame(1);
#else
	if (i)
		goto setup;
	i = getopt_int("", "mode");
	if (i & MODE_NETWORK)
		i = 3;
	else
		i = 2-!(i & MODE_2PLAYER);
# if NO_MENU
setup:	setupgame(i);
# else
	if (!in_menu)
		goto setup;
	mk_invitfile();
	while (1) {
		i = startupmenu(i);
		if (!i)
			break;
#  ifdef INET
		if (i==3 && !netplaymenu())
			continue;
#  endif
		if (i <= 3) {
setup:			setupgame(i);
#  ifdef INET
			if (i==3 && playerlist_n > 0 && netplaymenu())
				goto setup;
#  endif
		}
		if (i==OPTIONS)
			optionsmenu();
		if (i==HIGHSCORES)
			hiscorelist();
	}
# endif /* !NO_MENU */
#endif	/* TWOPLAYER */
	textgfx_end();
}

static void cleanup()
{
	timer_end();
	freeoptions("");
#if !NO_MENU
	rm_invitfile();
#endif
#ifdef SOCKET
	sock_flags &= ~CONNECTED;
	rmsocket();
#endif
#if ALLEGRO && WIN32 && !ALLEGRO_USE_CONSOLE
	fclose(stdout);
	delete_file("stdout.tmp");
#endif
}

static void finish(int sig)
{
	exit(0);
}

int main(int argc, char **argv)
{
	setcfgfilename(argv[0]);
	readoptions();
	timer_init();
	atexit(cleanup);
#if UNIX
	signal(SIGINT, finish);
#endif
	gettermsize();
	getlang();
	if (argc > 1) {
#if ALLEGRO && WIN32 && !ALLEGRO_USE_CONSOLE
		freopen("stdout.tmp", "w", stdout);
#endif
		proc_args(argv+1, argc-1);
	}
	gettermoptions();
#ifdef SOCKET
	startup(socket_fd>0 ? 2+is_inet() : 0);
#else
	startup(0);
#endif
#if !NO_MENU
	writeconfig_message();
#endif
	return 0;
}
END_OF_MAIN()
