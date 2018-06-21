#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <time.h>

/* Force same start level and height
#define SAME_LEVEL_HEIGHT 1
*/

#define MAX_CLIENTS 100

/* client flags */
#define FIRST_BYTE 1
#define AVAILABLE 2
#define MODE_BTYPE 4
#define IN_GAME 8

int listen_sock;

int num_clients = 0;	// largest used index + 1
int num_available = 0;

struct client {
	int sock;
	char name[16];	// no null byte at end
	char flags;
#ifdef SAME_LEVEL_HEIGHT
	char level;
	char height;
#endif
	char lineslimit;
	short opponent;
	char wins;
} clients[MAX_CLIENTS];

char fwd_buf[64];
int fwd_buf_n;

int bind_listen(int sock, uint16_t port)
{
	struct sockaddr_in addr;
	int len = sizeof(struct sockaddr_in);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(sock, (struct sockaddr *) &addr, len) < 0) {
		perror("Could not bind socket");
		return 0;
	}
	if (listen(sock, 10) < 0) {
		perror("listen");
		return 0;
	}
	return 1;
}

void printerror(const char *s)
{
	printf("ERROR %s: %s (errno=%d)\n", s, strerror(errno), errno);
}

void accept_conn(int sock)
{
	struct sockaddr_in name;
	socklen_t len = sizeof(name);
	sock = accept(sock, (struct sockaddr *) &name, &len);
	if (sock < 0) {
		printerror("accept");
		exit(1);
	}
	printf("player %d: ", num_clients+1);
	printf("connect from host %s\n", inet_ntoa(name.sin_addr));
	struct client *c = clients + num_clients;
	c->sock = sock;
	strncpy(c->name, inet_ntoa(name.sin_addr), 16);
	c->flags = FIRST_BYTE;
	c->opponent = -1;
}

int fd_set_clients(fd_set *fdset, int nfds)
{
	int i, fd;
	for (i=0; i < num_clients; i++) {
		fd = clients[i].sock;
		if (fd >= 0) {
			FD_SET(fd, fdset);
			if (fd >= nfds)
				nfds = fd+1;
		}
	}
	return nfds;
}

int waitinput_1sec(int sock)
{
	fd_set set;
	struct timeval tmv;
	FD_ZERO(&set);
	while (1) {
		FD_SET(sock, &set);
		tmv.tv_sec = 1;
		tmv.tv_usec = 0;
		if (select(sock+1, &set, 0, 0, &tmv) < 0) {
			if (errno == EINTR)
				continue;
			printerror("select");
			return 0;
		}
		return FD_ISSET(sock, &set);
	}
}

int readbytes(int sock, char *dest, int n)
{
	int i;
	if (!waitinput_1sec(sock))
		return 0;
	i = recv(sock, dest, n, 0);
	if (i == n)
		return 1;
	if (i < 0 && errno == EINTR)
		i = 0;
	else if (i <= 0)
		return 0;
	return readbytes(sock, dest+i, n-i);
}

/* read first byte of message */
int read_one_byte(int sock)
{
	char b;
	if (readbytes(sock, &b, 1) && b > 0 && (b < '\t' || b >= '\033'))
		return b;
	return 0;
}

int writebytes(int sock, const char *buf, int n)
{
	int i = send(sock, buf, n, 0);
	if (i == n)
		return 1;
	if (i < 0) {
		if (errno != EINTR) {
			printerror("send");
			return 0;
		}
		i = 0;
	}
	return writebytes(sock, buf+i, n-i);
}

void set_mode_str(int i, char *s)
{
	memset(s, 0, 3);
	if (clients[i].flags & MODE_BTYPE) {
		s[0] = 'B';
		int lines = clients[i].lineslimit;
		if (lines < 10)
			s[1] = '0'+lines;
		else {
			s[1] = '0'+lines/10;
			s[2] = '0'+lines%10;
		}
	}
}

void send_playerlist(int sock)
{
	char buf[20] = "PL";
	buf[2] = num_available;
	writebytes(sock, buf, 3);
	int i;
	for (i=0; i < num_clients; i++)
		if (clients[i].flags & AVAILABLE) {
			buf[0] = i+1;
			memcpy(buf+1, clients[i].name, 16);
			set_mode_str(i, buf+17);
			writebytes(sock, buf, 20);
		}
}

void set_available(int i, int sock)
{
	clients[i].flags |= AVAILABLE;
	num_available++;
	writebytes(sock, "\033P1", 3);
#ifdef SAME_LEVEL_HEIGHT
	clients[i].height = -1;
	writebytes(sock, "h_", 2);
#endif
}

void set_opponents(int i, int j)
{
	printf("%s vs. %s\n", clients[j].name, clients[i].name);
	clients[i].opponent = j;
	clients[i].wins = clients[j].wins = 0;
	clients[j].opponent = i;
	clients[j].flags &= ~AVAILABLE;
	num_available--;
	writebytes(clients[i].sock, "P2", 2);
#ifdef SAME_LEVEL_HEIGHT
	clients[i].level = -2;
	clients[j].level = -1;
	clients[i].height = clients[j].height;
#endif
}

void first_available_opponent(int i)
{
	int j;
	for (j=0; j < num_clients; j++)
		if (clients[j].flags & AVAILABLE) {
			set_opponents(i, j);
			break;
		}
}

int choose_opponent(int i, int j, const char *name)
{
	if (j >= 0 && j < num_clients && (clients[j].flags & AVAILABLE) &&
					!strncmp(clients[j].name, name, 16))
	{	set_opponents(i, j);
		return 1;
	}
	return 0;
}

void copy_name(char *dest, const char *name)
{
	int i;
	for (i=0; i < 16; i++) {
		if (*name >= ' ' && *name <= '~') {
			*dest = *name;
			dest++;
		}
		name++;
	}
}

int send_opponent_name(int i)
{
	char buf[17] = "N";
	int sock = clients[i].sock;
	i = clients[i].opponent;
	if (i >= 0) {
		copy_name(buf+1, clients[i].name);
		return writebytes(sock, buf, 17);
	}
	return 1;
}

void get_client_name(int i, char *buf)
{
	if (memcmp(clients[i].name, buf, 16)) {
		buf[16] = '\0';
		printf("player %d: name %s\n", i+1, buf);
	}
	memcpy(clients[i].name, buf, 16);
}

void remove_client_elem(int i)
{
	if (i+1 == num_clients)
		num_clients--;
	else {
		clients[i].sock = -1;
		clients[i].flags = 0;
	}
}

int remove_client(int i)
{
	close(clients[i].sock);
	printf("player %d disconnected\n", i+1);
	if (clients[i].flags & AVAILABLE)
		num_available--;
	remove_client_elem(i);
	int j = clients[i].opponent;
	if (j >= 0) {
		if (fwd_buf_n) {
			writebytes(clients[j].sock, fwd_buf, fwd_buf_n);
			fwd_buf_n = 0;
		}
		clients[j].opponent = -1;
		if (num_available > 0)
			remove_client(j);
		else {
			set_available(j, clients[j].sock);
			if (i < j) {
				printf("player %d -> %d\n", j+1, i+1);
				clients[i] = clients[j];
				remove_client_elem(j);
				i--;
			}
		}
	}
	while (num_clients > 0 && clients[num_clients-1].sock < 0)
		num_clients--;
	return i;
}

#ifdef SAME_LEVEL_HEIGHT
void set_level_height(int i, int lev, int h)
{
	char s[3] = "l";
	s[1] = lev+'0';
	s[2] = h+'0';
	writebytes(clients[i].sock, s, 3);
}

int set_level(int i, int lev)
{
	int j = clients[i].opponent;
	if (lev >= 0 && lev <= 9 && j >= 0) {
		if (clients[j].flags & IN_GAME) {
			if (clients[j].level >= 0) {
				lev = clients[j].level;
				set_level_height(i, lev, clients[j].height);
			}
		}
		else if (lev != clients[i].level && clients[i].level > -2)
			set_level_height(j, lev, clients[i].height);
		clients[i].level = lev;
	}
	return lev;
}

int set_height(int i, int h)
{
	int j = clients[i].opponent;
	if (h >= 0 && h <= 5 && h != clients[i].height) {
		if (j >= 0) {
			if (clients[j].flags & IN_GAME) {
				h = clients[j].height;
				set_level_height(i, clients[j].level, h);
			} else
				set_level_height(j, clients[i].level, h);
		}
		clients[i].height = h;
	}
	return h;
}
#endif

void set_wins(int i, int w)
{
	int j = clients[i].opponent;
	if (j < 0) {
		clients[i].wins = 0;
		return;
	}
	if (w > 0) {
		if (w == 2) clients[i].wins++;
		if (w == 1) clients[j].wins++;
		if (clients[i].wins == 3 || clients[j].wins == 3) {
			printf("%s vs. %s %d-%d\n",
					clients[i].name, clients[j].name,
					clients[i].wins, clients[j].wins);
			w = 0;
		}
	}
	if (w == 0)
		clients[i].wins = clients[j].wins = 0;
}

int fwd_to_opponent(int i, char *buf, int n)
{
	if (buf[0] == 'm') {
		if (buf[1] & MODE_BTYPE) {
			if (buf[2] > 25)
				buf[1] &= ~MODE_BTYPE;
			if (buf[2] < 5)
				buf[2] = 5;
			writebytes(clients[i].sock, buf, 3);
		}
		clients[i].flags &= ~MODE_BTYPE;
		clients[i].flags |= buf[1] & MODE_BTYPE;
		clients[i].lineslimit = buf[2];
	}
#ifdef SAME_LEVEL_HEIGHT
	else if (buf[0] == 'p')
		buf[1] = set_level(i, buf[1]);
	else if (buf[0] == 'h' && buf[1] != '-')
		buf[1] = set_height(i, buf[1]-'0') + '0';
#endif
	else if (buf[0] == 'w')
		set_wins(i, buf[1]-'0');
	i = clients[i].opponent;
	if (i >= 0) {
		if (fwd_buf_n + n > 64) {
			if (!writebytes(clients[i].sock, fwd_buf, fwd_buf_n))
				return 0;
			fwd_buf_n = 0;
		}
		memcpy(fwd_buf+fwd_buf_n, buf, n);
		fwd_buf_n += n;
	}
	return 1;
}

int has_more_input(int sock)
{
	fd_set set;
	struct timeval tmv = {0};
	FD_ZERO(&set);
	FD_SET(sock, &set);
	return select(sock+1, &set, 0, 0, &tmv) == 0 && FD_ISSET(sock, &set);
}

int handle_message(int i, int sock, int b)
{
	char buf[48];
	int n;
	if (!b)
		return remove_client(i);
	buf[0] = b;
	n = 0;
	switch (b) {
	case 'C':
		if (!readbytes(sock, buf, 1))
			return remove_client(i);
		if (buf[0] == '1') {
			if (!readbytes(sock, buf, 17))
				return remove_client(i);
			if (choose_opponent(i, buf[0]-1, buf+1))
				return i;
			// else goto case 'L'
		} else {
			if (buf[0] == '0')
				set_available(i, sock);
			return i;
		}
	case 'L':
		if (clients[i].flags & AVAILABLE)
			return remove_client(i);
		if (num_available == 0)
			set_available(i, sock);
		else
			send_playerlist(sock);
		return i;
	case '\033':
		set_wins(i, 0);
		break;
	case 'G':
		clients[i].flags |= IN_GAME;
		break;
	case 'N':
		if (readbytes(sock, buf, 1)) {
			if (buf[0] == '_') {
				if (send_opponent_name(i))
					return i;
			} else if (readbytes(sock, buf+1, 15)) {
				get_client_name(i, buf);
				return i;
			}
		}
		return remove_client(i);
	case 'b':
	case 'g':
	case 'h':
	case 'w':
		n = 1;
		break;
	case 'p':
		clients[i].flags &= ~IN_GAME;
		writebytes(sock, "N_", 2);
	case 'm':
	case 'n':
		n = 2;
		break;
	case 'x':
		n = 4;
	}
	if (n && !readbytes(sock, buf+1, n) || !fwd_to_opponent(i, buf, n+1))
		return remove_client(i);
	if (b == 'b') {
		n = buf[1];
		if (n > 0 && n <= 12) {
			n *= 4;
			if (!readbytes(sock, buf, n) ||
			    !fwd_to_opponent(i, buf, n))
				i = remove_client(i);
		}
	}
	if (!has_more_input(sock))
		return i;
	return handle_message(i, sock, read_one_byte(sock));
}

void read_from_clients(fd_set *fdset)
{
	int i;
	int sock;
	int b;
	for (i=0; i < num_clients; i++) {
		sock = clients[i].sock;
		if (sock < 0 || !FD_ISSET(sock, fdset))
			continue;
		fwd_buf_n = 0;
		b = read_one_byte(sock);
		if (!b) {
			i = remove_client(i);
			continue;
		}
		if (clients[i].flags & FIRST_BYTE) {
			clients[i].flags = 0;
			if (b == 'i') {
				if (num_available == 0)
					set_available(i, sock);
				else
					first_available_opponent(i);
			} else if (b != 'L') {
				printf("player %d: invalid first byte x%X\n",
					i+1, b);
				i = remove_client(i);
				continue;
			}
		}
		i = handle_message(i, sock, b);
		if (fwd_buf_n)
			writebytes(clients[clients[i].opponent].sock,
				   fwd_buf, fwd_buf_n);
	}
}

void sighandler(int sig)
{
	int sock;
	if (sig != SIGPIPE) {
		close(listen_sock);
		while (num_clients > 0) {
			num_clients--;
			sock = clients[num_clients].sock;
			if (sock >= 0)
				close(sock);
		}
		exit(0);
	}
}

void write_logfile(const char *logfile)
{
	fclose(stdout);
	stdout = fopen(logfile, "a");
	if (!stdout) {
		perror("Could not open LOGFILE for writing");
		exit(1);
	}
}

int main(int argc, char **argv)
{
	int port = 34034;
	int sock;
	fd_set fdset;
	int nfds;
	printf("Usage: %s [PORT] [LOGFILE]\n", argv[0]);
	if (argc > 1) {
		port = atoi(argv[1]);
		if (port < 1)
			port = 1;
		if (port > 0xFFFF)
			port = 0xFFFF;
	}
	printf("PORT %d\n", port);
	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		perror("Could not create socket");
		return 1;
	}
	if (!bind_listen(sock, port)) {
		close(sock);
		return 1;
	}
	if (argc > 2) {
		fclose(stdout);
		stdout = fopen(argv[2], "w");
		if (!stdout) {
			perror("Could not open LOGFILE for writing");
			return 1;
		}
	}
	listen_sock = sock;
	signal(SIGINT, sighandler);
	signal(SIGTERM, sighandler);
	signal(SIGPIPE, sighandler);	// ignore

	time_t t = 0;
	while (1) {
		FD_ZERO(&fdset);
		FD_SET(sock, &fdset);
		nfds = sock+1;
		if (!num_clients)
			puts("Listening for connections...");
		else
			nfds = fd_set_clients(&fdset, nfds);
		if (select(nfds, &fdset, 0,0,0) < 0 && errno != EINTR) {
			printerror("select");
			return 1;
		}
		read_from_clients(&fdset);
		if (FD_ISSET(sock, &fdset) && num_clients < MAX_CLIENTS) {
			accept_conn(sock);
			num_clients++;
		}
		if (argc > 2)
			write_logfile(argv[2]);
		if (time(NULL)-t > 12*60*60) {
			time(&t);
			printf(ctime(&t));
		}
	}
	return 0;
}
