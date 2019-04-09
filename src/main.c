#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <stdio.h>

#ifdef ALLEGRO
#include "config.h"
#include <allegro.h>
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
#include "menu/menu.h"
#include "net/sock.h"

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
		bgdot = default_bgdot;
	if (in_menu)
		textgfx_entermenu();
	if (i)
		creategame();
	else {
		i = getopt_int("", "mode");
		if (i & MODE_NET)
			i = 3;
		else
			i = 2-!(i & MODE_2P);
		if (in_menu)
			mk_invitfile();
		else
			creategame();
	}
	/* main loop */
	while (1) {
		if (game.state != GAME_CREATED) {
			i = startupmenu(i);
			if (!i)
				break;
#ifdef INET
			if (i==3 && !netplaymenu())
				continue;
#endif
			if (i<=3)
				creategame();
			if (i==OPTIONS)
				optionsmenu();
			if (i==HIGHSCORES)
				hiscorelist();
			continue;
		}
		if (in_menu && !gamemenu()) {
			inputdevs_player[0] = 0;
#ifdef SOCKET
			rmsocket();
			mk_invitfile();
#endif
			game.state = GAME_NULL;
#ifdef INET
			if (i==3 && playerlist_n > 0 && netplaymenu())
				creategame();
#endif
			continue;
		}
		rm_invitfile();
		writeconfig();
		do textgfx_entergame();
		while (startgame());
		clearwin(0);
		textgfx_entermenu();
		readoptions();
	}
	textgfx_end();
}

static void cleanup()
{
	timer_end();
	freeoptions("");
	rm_invitfile();
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

void outofmem()
{
	textgfx_end();
	puts("OUT OF MEMORY!");
	exit(1);
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
	writeconfig_message();
	return 0;
}
END_OF_MAIN()
