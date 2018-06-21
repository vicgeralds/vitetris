#include <stdio.h>
#include <stdlib.h>
#include "../config.h"
#ifdef WIN32
#include <winsock.h>
#else
# ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
# endif
# include <sys/socket.h>
# include <netinet/in.h>
# include <netdb.h>
#endif
#include "internal.h"
#include "sock.h"

FILE *inet_out = NULL;

extern struct sockaddr *sock_addr;

static struct in_addr *get_h_addr(const char *name)
{
	struct hostent *h = gethostbyname(name);
	if (!h) {
		fprintf(inet_out, "Unknown host \"%s\"\n", name);
		return NULL;
	}
	return (struct in_addr *) h->h_addr;
}

static int getaddr(const char *name, unsigned short port)
{
	struct sockaddr_in *addr;
	struct in_addr *h;
	if (sock_addr)
		free(sock_addr);
	addr = malloc(sizeof(struct sockaddr_in));
	addr->sin_family = AF_INET;
	addr->sin_port = htons(port);
	if (name) {
		h = get_h_addr(name);
		if (!h)
			return 0;
		addr->sin_addr = *h;
	} else
		addr->sin_addr.s_addr = htonl(INADDR_ANY);
	sock_addr = (struct sockaddr *) addr;
	return 1;
}

int mksocket_inet(const char *name, unsigned port)
{
	char *msg = mksocket(PF_INET);
	if (!inet_out)
		inet_out = stdout;
	if (msg)
		goto err;
	if (!getaddr(name, port))
		return 0;
	if (!name) {
		msg = bind_listen(sizeof(struct sockaddr_in));
		if (msg)
			goto err;
		fprintf(inet_out, "Listening for connections on "
				  "port %d...\n", port);
		return 1;
	}
	fprintf(inet_out, "Connecting to %s on port %d...\n", name, port);
	if (connectsock(sizeof(struct sockaddr_in)))
		return 1;
	fprintf(inet_out, "FAILED! %s\n", sock_strerror());
	return 0;

err:	fprintf(inet_out, "%s\n", msg);
	free(msg);
	return 0;
}

int reconnect_inet()
{
	char *msg;
	if (is_inet()) {
		msg = mksocket(PF_INET);
		if (msg)
			free(msg);
		else if (connectsock(sizeof(struct sockaddr_in)))
			return 1;
		sock_flags = CONNECTED | CONN_BROKEN;
	}
	return 0;
}

int is_inet()
{
	return sock_addr && sock_addr->sa_family == AF_INET;
}
