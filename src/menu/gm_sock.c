#include <string.h>
#include <stdlib.h>
#include "menu.h"
#include "../input/input.h"
#include "../textgfx/textgfx.h"
#include "../draw/draw.h"
#include "../game/tetris.h"
#include "../game/game.h"
#include "../netw/sock.h"
#undef SOCKET
#define SOCKET 1
#include "internal.h"

int yesno_menu(int x, int y)
{
	const char *menu[2] = {"Yes", "No"};
	int i = 0;
	int xx;
	while (1) {
		setcurs(x, y);
		printmenuitem("Yes", !i);
		printmenuitem("No", i);
		xx = i ? x+6 : x;
		setcurs(xx, y);
		refreshwin(-1);
		switch (handle_menuitem_2cols(menu, 2, &i, 1, xx, y, NULL,
					getkeypress_block(1 | SINGLE_PL))) {
		case 0:
			return 0;
		case 2:
			return !i;
		}
	}
}

static void printplayerlist(const char **menu, int n, int pos, int off,
			    int x, int y)
{
	const char *s;
	int i;
	setcurs(x, y);
	for (i=0; i<n; i++) {
		cleartoeol();
		printmenuitem(menu[i], i==pos);
		if (i < n-2) {
			s = playerlist[i+off].mode;
			if (s[0]) {
				setattr_standout();
				printstr(s);
				setattr_normal();
				putch(' ');
			}
			s = get_wonlost_stats(my_name, menu[i]);
			if (strcmp(s, "0-0"))
				printstr(s);
		}
		if (i==0 && off || i==9 && playerlist_n - off > 10) {
			setcurs(x+16, y+i);
			printstr(" (more) ");
		}
		newln(x);
	}
}

static int playerlist_menu(int n, int x, int y)
{
	const char *menu[12];
	int i, off;
	int keypr = 0;
	menu[n]   = "-";
	menu[n+1] = "Wait for other";
	n += 2;
	i = off = 0;
	while (1) {
		if (i==0) {
			for (i=0; i<n-2; i++)
				menu[i] = playerlist[i+off].name;
			i = 0;
		}
		printplayerlist(menu, n, i, off, x, y);
		setcurs(x, y+i);
		refreshwin(-1);
		keypr = getkeypress_block(1) & 0xFF;
		if (sock_flags & PLIST_UNHANDLED)
			return 0;
		if (sock_flags & CONN_BROKEN || !playerlist)
			return 1;
		switch (keypr) {
		case MVUP:
			if (i==0 && off) {
				off--;
				continue;
			}
			break;
		case '\t':
			if (i==11 && off) {
				i = off = 0;
				continue;
			}
		case MVDOWN:
			if (i==9 && i+off+1 < playerlist_n) {
				off++;
				memmove(menu, menu+1, 9*sizeof(char*));
				menu[9] = playerlist[i+off].name;
				continue;
			}
			break;
		case '.':
			continue;
		}
		switch (handle_menuitem(menu, n, &i, x, y+i, NULL, keypr)) {
		case 2:
			if (i < n-2) {
				connect_to_player(playerlist+i+off);
				if (waitinput_sock(500))
					sock_getkeypress(0);
				return !(sock_flags & PLIST_UNHANDLED);
			}
			connect_to_player(NULL);
			return 1;
		case 0:
			rmsocket();
			return 1;
		}
	}
}

static void show_playerlist(int x, int y)
{
	int n;
	while (playerlist) {
		sock_flags &= ~PLIST_UNHANDLED;
		n = 10;
		if (playerlist_n < 10)
			n = playerlist_n;
		setcurs(x, y);
		printstr("Players:");
		if (playerlist_menu(n, x+10, y) && playerlist) {
			free(playerlist);
			playerlist = NULL;
		}
		clearbox(x, y, 0, n+2);
	}
}

int gamemenu_socket(const char **menu, int i, int x, int y, menuhandler *hs)
{
	int keypr = 0;
	clearbox(x, y, 0, 7);
	/* remove separator */
	memmove(menu+2, menu+3, 3*sizeof(*menu));
	memmove(hs+2, hs+3, 3*sizeof(*hs));
	if (i>2)
		i--;
	while (1) {
		if (!keypr)	/* redraw once every second */
			drawmenu(menu, 5, i, x, y, hs);
		setcurs(x, y+5);
		newln(x);
		if (!(sock_flags & CONNECTED)) {
			if (sock_flags & CONN_PROXY) {
				if (playerlist) {
					clearbox(x, y, 0, 12);
					show_playerlist(x, y);
					if (socket_fd > -1) {
						keypr = 0;
						continue;
					}
				}
				clearbox(x, y+6, 0, 6);
				if (socket_fd < 0)
					return 0;
				setcurs(x, y+6);
			}
			printstr("Waiting for opponent...");
		} else {
			sock_sendplayer();
			sock_sendbyte('h'); /* request height */
			sock_sendbyte('-');

			setattr_bold();
			printstr("PLAYER 2");
			cleartoeol();
			setattr_normal();
			newln(x+4);
			printstr("Level:  ");
			putch(player2.startlevel+'0');
			movefwd(4);
			printstr("Height:  ");
			putch(player2.height+'0');
		}
		newln(x);
		if (game->mode & MODE_BTYPE) {
			cleartoeol();
			newln(x);
			setattr_bold();
			printint("B-TYPE  LINES %d", player1.lineslimit);
			setattr_normal();
			newln(x);
			if (opponent_name[0])
				newln(x);
		} else {
			newln(x);
			cleartoeol();
		}
		if (opponent_name[0]) {
			if (my_name[0])
				printstr(my_name);
			else
				printstr("You");
			putch(' ');
			printstr(get_wonlost_stats(my_name, opponent_name));
			putch(' ');
			printstr(opponent_name);
			cleartoeol();
			newln(x);
		}
		setcurs(x, y+i);
		refreshwin(-1);
		keypr = getkeypress(1000, 0);
		if (!keypr)
			continue;
		switch (handle_menuitem(menu, 5, &i, x, y+i, hs, keypr)) {
		case 0:
			if (CONNECTED == (sock_flags & (CONNECTED |
							CONN_BROKEN))) {
				drawbox(x, y, 28, 6, NULL);
				setcurs(x+2, y+1);
				printstr("Are you sure you want to");
				newln(x+2);
				printstr("close the connection?");
				if (!yesno_menu(x+4, y+4) ||
				    playerlist_n > 0 && reconnect_server()) {
					clearbox(x, y, 28, 6);
					keypr = 0;
					continue;
				}
			}
			i = 1;
			if (game->mode & MODE_BTYPE)
				i += 2;
			if (opponent_name[0])
				i += 2;
			clearbox(0, y+7, 0, i);
			return 0;
		case 2:
			player2.height = 0;
			if (i>1)
				i++;
			return i+1;
		}
	}
}
