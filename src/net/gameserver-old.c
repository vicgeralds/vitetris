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

#define MAX_CLIENTS 100

int num_clients = 0;

struct client {
	int sock;
	char name[16];	// no null byte at end
} clients[MAX_CLIENTS];

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

int writebytes(int sock, const char *buf, int n)
{
	int i = send(sock, buf, n, 0);
	if (i == n)
		return 1;
	if (i < 0) {
		if (errno != EINTR) {
			perror("send");
			return 0;
		}
		i = 0;
	}
	return writebytes(sock, buf+i, n-i);
}

void accept_conn(int sock)
{
	struct sockaddr_in name;
	socklen_t len = sizeof(name);
	sock = accept(sock, (struct sockaddr *) &name, &len);
	if (sock < 0) {
		perror("accept");
		exit(1);
	}
	printf("connect from host %s\n", inet_ntoa(name.sin_addr));
	clients[num_clients].sock = sock;
	strncpy(clients[num_clients].name, inet_ntoa(name.sin_addr), 16);
	writebytes(sock, !(num_clients % 2) ? "P1" : "P2", 2);
}

int fd_set_clients(fd_set *fdset, int nfds)
{
	int i, fd;
	for (i=0; i < num_clients; i++) {
		fd = clients[i].sock;
		FD_SET(fd, fdset);
		if (fd >= nfds)
			nfds = fd+1;
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
			perror("select");
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

void send_opponent_name(int i)
{
	char buf[17] = "N";
	int sock = clients[i].sock;
	if (i % 2 == 0)
		i++;
	else
		i--;
	if (i < num_clients) {
		copy_name(buf+1, clients[i].name);
		writebytes(sock, buf, 17);
	}
}

int remove_client(int i)
{
	int sock = clients[i].sock;
	close(sock);
	num_clients--;
	printf("player %d disconnected\n", i+1);
	if (i % 2 == 0)
		sock = clients[i+1].sock;
	else
		sock = clients[--i].sock;
	if (i < num_clients) {
		memmove(clients+i, clients+i+2,
			(num_clients-i-1)*sizeof(struct client));
		if (num_clients % 2 == 0) {
			close(sock);
			num_clients--;
		} else {
			clients[num_clients-1].sock = sock;
			writebytes(sock, "\033P1", 3);
		}
	}
	return i-1;
}

int fwd_to_opponent(int i, const char *buf, int n)
{
	if (i % 2 == 0)
		i++;
	else
		i--;
	return i == num_clients || writebytes(clients[i].sock, buf, n);
}

void read_from_clients(fd_set *fdset)
{
	char buf[48];
	int i;
	int sock;
	int n;
	for (i=0; i < num_clients; i++) {
		sock = clients[i].sock;
		if (!FD_ISSET(sock, fdset))
			continue;
		if (!readbytes(sock, buf, 1) || buf[0] >= '\t' &&
						buf[0] < '\033') {
			i = remove_client(i);
			continue;
		}
		n = 0;
		switch (buf[0]) {
		case 'N':
			if (readbytes(sock, buf+1, 1)) {
				if (buf[1] == '_') {
					send_opponent_name(i);
					continue;
				}
				if (readbytes(sock, buf+2, 15)) {
					memcpy(clients[i].name, buf+1, 16);
					continue;
				}
			}
			i = remove_client(i);
			continue;
		case 'b':
		case 'g':
		case 'h':
		case 'w':
			n = 1;
			break;
		case 'p':
			writebytes(sock, "N_", 2);
		case 'm':
		case 'n':
			n = 2;
			break;
		case 'x':
			n = 4;
		}
		if (n && !readbytes(sock, buf+1, n) ||
		    !fwd_to_opponent(i, buf, n+1))
			i = remove_client(i);
		else if (buf[0] == 'b') {
			n = buf[1];
			if (n > 0 && n <= 12) {
				n *= 4;
				if (!readbytes(sock, buf, n) ||
				    !fwd_to_opponent(i, buf, n))
					i = remove_client(i);
			}
		}
	}
}

void sighandler(int sig)
{
	if (sig == SIGINT)
		num_clients = MAX_CLIENTS+1;
}

int main(int argc, char **argv)
{
	int port = 34034;
	int sock;
	fd_set fdset;
	int nfds;
	printf("Usage: %s [PORT]\n", argv[0]);
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
	signal(SIGINT, sighandler);
	signal(SIGPIPE, sighandler);
	while (num_clients <= MAX_CLIENTS) {
		FD_ZERO(&fdset);
		FD_SET(sock, &fdset);
		nfds = sock+1;
		if (!num_clients)
			puts("Listening for connections...");
		else
			nfds = fd_set_clients(&fdset, nfds);
		if (select(nfds, &fdset, 0,0,0) < 0 && errno != EINTR) {
			perror("select");
			return 1;
		}
		read_from_clients(&fdset);
		if (FD_ISSET(sock, &fdset) && num_clients < MAX_CLIENTS) {
			accept_conn(sock);
			num_clients++;
		}
	}
	close(sock);
	return 0;
}
