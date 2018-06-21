#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../config.h"
#ifdef WIN32
#include <winsock.h>
#else
# ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
# endif
# include <sys/socket.h>
# include <signal.h>
# include <errno.h>
#endif
#ifdef UNIX
#include <sys/un.h>	/* sockaddr_un */
#include <unistd.h>	/* unlink */
#endif
#include "sock.h"
#include "internal.h"
#include "../input/input.h"

#ifdef WIN32
#define TEST_EINTR (WSAGetLastError() == WSAEINTR)
#else
#define TEST_EINTR (errno == EINTR)
#endif

int sock_flags = -1;
struct sockaddr *sock_addr = NULL;

#ifdef WIN32
static int winsock_started = 0;
static SOCKET mysock = INVALID_SOCKET;
#else
typedef int SOCKET;
#define mysock socket_fd
#endif

void conn_broken(int sig)
{
	sock_flags = CONN_BROKEN | sock_flags & ~PL2_IN_GAME;
}

const char *sock_strerror()
{
#ifndef WIN32
	return strerror(errno);
#else
	if (winsock_started)
	switch (WSAGetLastError()) {
	case WSAENETDOWN:	return "Network is down";
	case WSAEADDRINUSE:	return "Address already in use";
	case WSAECONNREFUSED:	return "Connection refused";
	case WSAETIMEDOUT:	return "Connection timed out";
	}
	return "";
#endif
}

char *sock_errmsg(const char *descr)
{
	int n = strlen(descr);
	const char *s = sock_strerror();
	char *msg     = malloc(n+strlen(s)+10);
	strcpy(msg, "ERROR! ");
	strcat(msg, descr);
	if (*s) {
		n += 7;
		msg[n] = ':';
		msg[n+1] = ' ';
		strcpy(msg+n+2, s);
	}
	return msg;
}

static void closesock()
{
	if (mysock != INVALID_SOCKET) {
#ifdef WIN32
		closesocket(mysock);
#else
		close(socket_fd);
#endif
		mysock = INVALID_SOCKET;
	}
}

char *mksocket(int domain)
{
	SOCKET sock;
#ifdef WIN32
	WSADATA info;
	if (!winsock_started && WSAStartup(MAKEWORD(1,1), &info) != 0)
		return errmsg("Winsock startup failed");
	winsock_started = 1;
#endif
	closesock();
	sock = socket(domain, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
		return errmsg("Could not create socket");
	mysock = sock;
#ifdef WIN32
	socket_fd = 1;
#endif
#ifdef SIGPIPE
	if (sock_flags == -1)
		signal(SIGPIPE, conn_broken);
#endif
	sock_flags = 0;
	return NULL;
}

char *bind_listen(int size)
{
	SOCKET sock = mysock;
	if (bind(sock, sock_addr, size) == -1)
		return errmsg("Could not bind socket");
	if (listen(sock, 1) == -1)
		return errmsg("listen");
	sock_flags |= IS_SERVER;
	return NULL;
}

int connectsock(int size)
{
	if (connect(mysock, sock_addr, size) == -1)
		return 0;
	sock_flags = CONNECTED;
	return 1;
}

void accept_conn()
{
	SOCKET sock = accept(mysock, NULL, NULL);
	closesock();
	mysock = sock;
	sock_flags |= CONNECTED;
}

void writebytes(const char *buf, int n)
{
	int i = send(mysock, buf, n, 0);
	if (i == n)
		return;
	if (i == -1) {
		if (!TEST_EINTR)
			return;
		i = 0;
	}
	writebytes(buf+i, n-i);
}

int waitinput_sock(unsigned msec)
{
#ifndef WIN32
	return waitinput(socket_fd, msec);
#else
	fd_set set;
	struct timeval tmv;
	int ret;
	if (mysock == INVALID_SOCKET)
		return 0;
	do {
		FD_ZERO(&set);
		FD_SET(mysock, &set);
		tmv.tv_sec = 0;
		tmv.tv_usec = 1000*msec;
		ret = select(0, &set, NULL, NULL, &tmv);
	} while (ret == -1 && TEST_EINTR);
	return ret > 0;
#endif
}

static int waitinput_1sec()
{
	int i;
	for (i=0; i<10; i++)
		if (waitinput_sock(100))
			return 1;
	return 0;
}

int readbytes(char *buf, int n)
{
	int i;
	if (sock_flags & CONN_BROKEN)
		return 0;
	if (!waitinput_1sec())
		goto broken;
	i = recv(mysock, buf, n, 0);
	if (i == n)
		return 1;
	if (i == -1 && TEST_EINTR)
		i = 0;
	else if (i <= 0) {
broken:		conn_broken(0);
		return 0;
	}
	return readbytes(buf+i, n-i);
}

void rmsocket()
{
	if (sock_addr) {
#ifdef UNIX
		if (is_server && sock_addr->sa_family == AF_UNIX)
			unlink(SUN_PATH(sock_addr));
#endif
		if (sock_flags & (CONN_PROXY | CONNECTED) !=
				 (CONN_PROXY | CONNECTED)) {
			free(sock_addr);
			sock_addr = NULL;
		}
	}
	closesock();
	socket_fd = -1;
#ifdef WIN32
	if (winsock_started) {
		WSACleanup();
		winsock_started = 0;
	}
#endif
	if (playerlist) {
		free(playerlist);
		playerlist = NULL;
	}
}
