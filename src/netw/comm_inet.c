#include <stdlib.h>
#include <string.h>
#include "sock.h"
#include "../game/tetris.h"

struct player_id *playerlist = NULL;
int playerlist_n = 0;

void request_playerlist()
{
	if ((sock_flags & (IS_SERVER | CONN_PROXY)) != IS_SERVER) {
		if (playerlist) {
			free(playerlist);
			playerlist = NULL;
		}
		playerlist_n = -1;
		sock_sendbyte('L');
	}
}

void connect_to_player(struct player_id *p)
{
	char s[19] = "C";
	if (!p) {
		s[1] = '0';
		writebytes(s, 2);
	} else {
		s[1] = '1';
		s[2] = p->id;
		memcpy(s+3, p->name, 16);
		writebytes(s, 19);
		sock_sendbyte('i');
		sock_sendplayer();
	}
}

int reconnect_server()
{
	char b;
	if (sock_flags & CONN_PROXY && reconnect_inet()) {
		sock_flags = CONN_PROXY;
		if (playerlist_n)
			request_playerlist();
		sock_initgame();
		return 1;
	}
	return 0;
}

static int recvplayerlist()
{
	struct player_id *p;
	char s[20];
	int n;
	if (readbytes(s, 1))
		n = s[0];
	if (n > 0) {
		playerlist_n = n;
		if (playerlist)
			free(playerlist);
		playerlist = malloc(n * sizeof(struct player_id));
		p = playerlist;
		if (p) {
			while (readbytes(s, 20)) {
				p->id = (unsigned char) s[0];
				memcpy(p->name, s+1, 16);
				p->name[16] = '\0';
				memcpy(p->mode, s+17, 3);
				p->mode[3] = '\0';
				p++;
				n--;
				if (!n)
					return 1;
			}
			request_playerlist();
		}
	}
	return 0;
}

static void send_mode()
{
	char s[3] = "m";
	s[1] = game->mode;
	s[2] = player1.lineslimit;
	writebytes(s, 3);
}

int handle_server_message()
{
	char b;
	if (readbytes(&b, 1)) {
		if (b =='L') {
			sock_flags = CONN_PROXY;
			if (!recvplayerlist())
				return 0;
			sock_flags |= PLIST_UNHANDLED;
		} else if (b =='1') {
			sock_flags = CONN_PROXY | IS_SERVER;
			sock_initgame();
			send_mode();
			if (playerlist) {
				free(playerlist);
				playerlist = NULL;
			}
		} else if (b =='2') {
			sock_flags = CONN_PROXY | CONNECTED;
			if (waitinput_sock(500) || waitinput_sock(500)) {
				sock_getkeypress(0);
				sock_sendplayer();
			}
		}
		return !game_over;
	}
	return 0;
}
