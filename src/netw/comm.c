#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../game/tetris.h"
#include "sock.h"
#include "../game/tetris2p.h"
#include "../input/input.h"
#include "../draw/draw.h"

int handle_server_message();	/* comm_inet.c */

char my_name[17] = "";
char opponent_name[17] = "";

static int pos_correct;

void sock_sendbyte(char b)
{
	writebytes(&b, 1);
}

void sock_initgame()
{
	static char saved_mode;
	static char saved_lineslimit;
	opponent_name[0] = '\0';
	if (sock_flags & CONN_PROXY) {
		game->mode = saved_mode;
		player1.lineslimit = saved_lineslimit;
	} else {
		saved_mode = game->mode;
		saved_lineslimit = player1.lineslimit;
		if (is_server)
			playerlist_n = 0;
	}
	if (is_server)
		player2.lineslimit = player1.lineslimit;
	else {
		sock_sendbyte('i');
		sock_sendplayer();
		if (waitinput_sock(500) || waitinput_sock(500))
			sock_getkeypress(0);
	}
}

static void initgame_response()
{
	char s[3] = "m";
	sock_sendplayer();
	s[1] = game->mode;
	s[2] = player1.lineslimit;
	writebytes(s, 3);
}

void sock_sendplayer()
{
	char s[3] = "p";
	s[1] = player1.startlevel;
	s[2] = player1.rotationsys;
	writebytes(s, 3);
}

static void recvplayer()
{
	char s[2];
	if (readbytes(s, 2)) {
		if (s[0] > 9 || s[0] < 0) {
			sock_flags |= CONN_BROKEN;
			return;
		}
		player2.startlevel = s[0];
		player2.rotationsys = s[1];
		writebytes("N_", 2);
	}
}

static void set_level_height(int flags)
{
	char s[2];
	int n;
	if (readbytes(s, 2) && !(flags & IN_GAME) && !game_over) {
		n = s[0]-'0';
		if (n >= 0 && n <= 9)
			player1.startlevel = n;
		n = s[1]-'0';
		if (n >= 0 && n <= 5)
			player1.height = n;
	}
}

static void sendname()
{
	char s[17] = "N";
	int i = 0;
	while (my_name[i] == '_')
		i++;
	if (my_name[i]) {
		strncpy(s+1, my_name+i, 16);
		writebytes(s, 17);
	}
}

static int getkey_cancel()
{
	switch (getkeypress(500, SINGLE_PL)) {
	case ESC:
	case ESC | PLAYER_2:
	case '\b':
		return 1;
	case 'q':
		exit(0);
	}
	return sock_flags & CONN_BROKEN;
}

int sock_wait_pl2ingame()
{
	print_game_message(1, "WAIT", 1);
	game->next = NULL;
	while (!(sock_flags & CONNECTED))
		if (getkey_cancel())
			return 0;
	writebytes("N_G", 3);
	sock_flags = WAIT_PL2INGAME | (sock_flags & ~SAME_HEIGHT);
	while (!(sock_flags & PL2_IN_GAME)) {
		if ((sock_flags & (CONN_PROXY | WAIT_PL2INGAME))
				== CONN_PROXY)
			return sock_wait_pl2ingame();
		if (getkey_cancel())
			break;
	}
	sock_flags &= ~WAIT_PL2INGAME;
	return sock_flags & PL2_IN_GAME;
}

static void sendboard(int n)
{
	char s[50] = "b";
	unsigned char *b;
	uint_least32_t row;
	int i, j;
	if (!(sock_flags & PL2_IN_GAME))
		return;
	s[1] = n;
	b = (unsigned char *) s + 2;
	i = 20;
	while (n) {
		row = player1.board[--i];
		j = 0;
		do {
			b[j] = row & 0xFF;
			j++;
		} while (row >>= 8);
		b += 4;
		n--;
	}
	writebytes(s, (char *)b - s);
}

static int p1_height_lines()
{
	int n = player1.height;
	switch (n) {
	case 0:
		break;
	case 1:
	case 2:
		n = 2*n+1;
		break;
	default:
		n = 2*(n+1);
	}
	return n;
}

static void p1_lines_height(int n)
{
	n /= 2;
	if (n > 3)
		n--;
	player1.height = n;
}

static void copy_same_height(int n)
{
	if (is_server)
		memcpy(player2.board, player1.board, 20*sizeof(uint_least32_t));
	else {
		memcpy(player1.board, player2.board, 20*sizeof(uint_least32_t));
		redrawboard(&player1, 19);
		sendboard(n);
	}
	sock_flags |= SAME_HEIGHT;
}

static int recvboard_read(char *s, int n)
{
	while (n > 0) {
		if (!readbytes(s, 4))
			return 0;
		s += 4;
		n--;
	}
	return 1;
}

static void recvboard()
{
	char s[48];
	unsigned char *b = (unsigned char *) s;
	uint_least32_t row;
	int n, m = 0;
	int i = 20;
	int j;
	if (!readbytes(s, 1)) 
		return;
	n = *s;
	if (n < 1 || n > 12) {
		conn_broken(0);
		return;
	}
	if (!recvboard_read(s, n) || !tetrom_seq)
		return;
	if (!game->state)
		m = p1_height_lines();
	if (!player1.height && sock_flags & SAME_HEIGHT) {
		m = n;
		p1_lines_height(n);
	}
	while (n) {
		row = b[3];
		for (j = 2; j >= 0; j--) {
			row <<= 8;
			row |= b[j];
		}
		player2.board[--i] = row;
		b += 4;
		n--;
	}
	n = 20-i;
	if (n == m)
		copy_same_height(n);
	redrawboard(&player2, 19);
	if (!game_running)
		print_press_key();
}

void sock_initround()
{
	char s[3] = "n1";
	sock_flags &= ~PL2_GARBAGE;
	sock_sendbyte('G');
	if (is_server) {
		s[2] = *tetrom_seq;
		writebytes(s, 3);
		s[1] = '2';
		writebytes(s, 3);
		s[2] = tetrom_seq[1];
		writebytes(s, 3);
		s[1] = '1';
		writebytes(s, 3);
	}
	if (player1.height)
		sendboard(p1_height_lines());
}

void sock_sendnext(const struct player *p, char n)
{
	char s[3] = "n";
	if (sock_flags & PL2_IN_GAME) {
		s[1] = '1'+(p == game->player);
		s[2] = n;
		writebytes(s, 3);
	}
}

static void recvnext()
{
	struct tetr next;
	char s[2];
	char *p = tetrom_seq;
	if (!p || !readbytes(s, 2) || s[1] > 6)
		return;
	switch (*s) {
	case '2':
		p += 2;
	case '1':
		break;
	default:
		return;
	}
	if (*p != 0x7F)
		p++;
	else {
		gettetrom(&next, s[1]);
		drawnext(game->player+(*s=='2'), &next);
		if (game->next)
		    *game->next = next;
	}
	*p = s[1];
}

void sock_sendpiece(const struct player *p)
{
	char s[5] = "x";
	if (sock_flags & PL2_IN_GAME) {
		int bl = player1.piece.blocks;
		s[1] = bl & 0xFF;
		s[2] = bl>>8;
		memcpy(s+3, &player1.piece.x, 2);
		writebytes(s, 5);
	}
}

static void recvpiece()
{
	struct player *p = &player2;
	struct tetr t;
	char bl[2];
	if (!readbytes(bl, 2) || !game_running)
		return;
	if (tetris2p[1].delay) {
		if (*tetris2p[1].clearedlines) {
			redrawboard(&player2, 19);
			*tetris2p[1].clearedlines = 0;
		}
		if (!p->piece.blocks)
			nextpiece_2p(p);
	}
	memcpy(&t, &p->piece, sizeof(struct tetr));
	p->piece.blocks = bl[0] | bl[1]<<8;
	readbytes(&p->piece.x, 2);
	if (memcmp(&t, &p->piece, sizeof(struct tetr))) {
		clearblocks(p, t.blocks, t.x, t.y);
		drawpiece(p,  p->piece.blocks, p->piece.x, p->piece.y);
	}
	pos_correct = 1;
}

void sock_sendgarbage_num(int n)
{
	char s[2] = "g";
	s[1] = n;
	writebytes(s, 2);
}

static int recvgarbage_num(int n)
{
	if (n <= 12)
		sendboard(n);
	else {
		tetris2p[1].garbage[1] = n-12;
		sock_flags |= PL2_GARBAGE;
		if (!tetris2p[1].delay || tetris2p[1].falltm) {
			getgarbage_player2();
			return 1;
		}
	}
	return 0;
}

void sock_sendwinner()
{
	char s[2] = "w";
	if (winner == &player1)
		s[1] = '2';
	else if (winner == &player2)
		s[1] = '1';
	else
		s[1] = '3';
	writebytes(s, 2);
}

int sock_getkeypress(int flags)
{
	signed char b;
	if (!waitinput_sock(0))
		return 0;
	if (!(sock_flags & (CONNECTED | CONN_PROXY))) {
		accept_conn();
		return 0;
	}
	if ((sock_flags & CONN_BROKEN) && !(flags & IN_GAME) && !game_over) {
		if (reconnect_server())
			return '.';
		return ESC;
	}
	if (playerlist && (sock_flags & PLIST_UNHANDLED)) {
		if (sock_flags & CONNECTED)
			conn_broken(0);
		else if (!game_over)
			return ESC;
	}
	if (!readbytes(&b, 1))
		return 0;
	if (b=='G' && game_over) {
		sock_flags &= ~PL2_IN_GAME;
		sock_sendbyte(ESC);
		goto out;
	}
	if (pos_correct == 2)
		pos_correct = 0;
	if (pos_correct)
		pos_correct = 2;
	switch (b) {
	case ESC:
		sock_flags &= ~PL2_IN_GAME;
		break;
	case 'G':
		if (!(sock_flags & WAIT_PL2INGAME))
			break;
		sock_flags |= PL2_IN_GAME;
	case '.':
		return '.';
	case 'N':
		if (readbytes(&b, 1)) {
			if (b =='_')
				sendname();
			else  {
				opponent_name[0] = b;
				readbytes(opponent_name+1, 15);
			}
		}
		break;
	case 'P':
		if (handle_server_message())
			return '.';
		break;
	case 'b':
		recvboard();
		break;
	case 'g':
		if (readbytes(&b, 1) && b > 0 && recvgarbage_num(b))
			return '.';
		break;
	case 'h':
		if (readbytes(&b, 1)) {
			if (isdigit(b))
				player2.height = b-'0';
			else {
				sock_sendbyte('h');
				sock_sendbyte(player1.height+'0');
			}
		}
		break;
	case 'i':
		if (sock_flags == (CONN_PROXY | IS_SERVER))
			sock_flags = CONN_PROXY | IS_SERVER | CONNECTED;
		if (is_server)
			initgame_response();
		break;
	case 'l':
		set_level_height(flags);
		break;
	case 'm':
		if (!(sock_flags & CONN_PROXY))
			playerlist_n = 0;
		readbytes(&game->mode, 1);
		readbytes(&player1.lineslimit, 1);
		player2.lineslimit = player1.lineslimit;
		break;
	case 'n':
		recvnext();
		break;
	case 'p':
		sock_flags &= ~PL2_IN_GAME;
		recvplayer();
		break;
	case 's':
		if (game_over)
			return '\n';
		break;
	case 'w':
		if (readbytes(&b, 1) && b >'0' && b <= '3') {
			winner = game->player + (b-'1');
			return '.';
		}
		break;
	case 'x':
		recvpiece();
		break;
	default:
		if (b >= '\t' && b < ESC)
			conn_broken(0);
		if (!(flags & IN_GAME))
			break;
				     /* for backward compatibility */
		if (b == HARDDROP || pos_correct && b == MVDOWN &&
				     hitbtm(&player2.piece, &player2))
			return HARDDROP;
		if (b > 0 && b <= B_BTN) {
			if (tetris2p[1].delay && tetris2p[1].falltm)
				nextpiece_2p(&player2);
			return b;
		}
	}
out:	return sock_getkeypress(flags);
}
