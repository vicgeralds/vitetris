#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pwd.h>
#include <errno.h>
#include "sock.h"
#include "internal.h"

const struct invit *invit = NULL;
char this_tty[8] = "";
static int invit_fd = -1;

static int get_this_tty()
{
	char *s = ttyname(STDOUT_FILENO);
	int i;
	if (!s)
		return 0;
	if (!strncmp(s, "/dev/", 5))
		i = 5;
	else if (strlen(s) < 8)
		i = 0;
	else {
		i = strlen(s)-7;
		if (s[i] == '/')
			i++;
	}
	strcpy(this_tty, s+i);
	return 1;
}

static void getfname(char *name, const char *tty)
{
	int i;
	memcpy(name, "/tmp/vitetris-", 14);
	strncpy(name+14, tty, 7);
	name[21] = '\0';
	for (i = 14; i < 20 && name[i]; i++)
		if (name[i] == '/')
			name[i] = '-';
}

void mkinvitfile()
{
	char name[22];
	if (*this_tty || get_this_tty()) {
		getfname(name, this_tty);
		invit_fd = open(name, O_CREAT | O_RDONLY | O_NONBLOCK,
				S_IREAD | S_IWRITE | S_IWGRP | S_IWOTH);
	}
}

static int getowner(const char *tty, char *username)
{
	char fname[22];
	struct stat st;
	struct passwd *pw;
	getfname(fname, tty);
	if (stat(fname, &st) == -1)
		return 0;
	if (pw = getpwuid(st.st_uid)) {
		strncpy(username, pw->pw_name, 15);
		username[15] = '\0';
		return 1;
	}
	return 0;
}

int checkinvit()
{
	static struct invit inv;
	int i = 0;
	ssize_t n;
	if (invit || invit_fd == -1)
		return 0;
	memset(inv.tty, 0, 8);
	while (i < 8) {
		n = read(invit_fd, inv.tty+i, 1);
		if (n == -1 && (errno==EAGAIN || errno==EINTR))
			continue;
		if (n < 1)
			break;
		if (inv.tty[i] == '\n') {
			inv.tty[i] = '\0';
			break;
		}
		i++;
	}
	if (i==8 || !i && n<1 && lseek(invit_fd, 0, SEEK_CUR) > 0) {
		rminvitfile();
		mkinvitfile();
		i = 0;
	}
	if (i && getowner(inv.tty, inv.user)) {
		invit = &inv;
		return 1;
	}
	return 0;
}

void rminvitfile()
{
	char name[22];
	if (invit_fd > -1) {
		close(invit_fd);
		invit_fd = -1;
		getfname(name, this_tty);
		unlink(name);
	}
}

static int add_2p_tty(char *tty, const char *name)
{
	struct stat st;
	char fname[22];
	int i;
	getfname(fname, name);
	if (!islower(*name) || strchr(name, '.') || stat(fname, &st) < 0)
		return 0;
	if (time(NULL) - st.st_mtime > 3600) {
		unlink(fname);
		return 0;
	}
	strncpy(tty, name, 7);
	for (i = 1; i < 7 && tty[i]; i++)
		if (tty[i] == '-')
			tty[i] = '/';
	if (!strcmp(tty, this_tty)) {
		memset(tty, 0, 7);
		return 0;
	}
	return 1;
}

int get_2p_ttys(char *ttys, int n)
{
	DIR *d = opendir("/tmp");
	struct dirent *e;
	int i;
	if (!d)
		return 0;
	memset(ttys, 0, 8*n);
	i = 0;
	while (i < n && (e = readdir(d))) {
		if (strncmp(e->d_name, "vitetris-", 9))
			continue;
		if (add_2p_tty(ttys+8*i, e->d_name+9))
			i++;
	}
	closedir(d);
	return i > 0;
}

static void getaddr(const char *tty, int server)
{
	char fname[29];
	struct sockaddr_un *addr;
	if (server) {
		tty = this_tty;
		if (!*tty)
			get_this_tty();
	}
	getfname(fname, tty);
	strcat(fname+14, ".socket");
	if (sock_addr)
		free(sock_addr);
	addr = malloc(offsetof(struct sockaddr_un, sun_path)
					+ strlen(fname)+1);
	addr->sun_family = AF_UNIX;
	strcpy(addr->sun_path, fname);
	sock_addr = (struct sockaddr *) addr;
}

static char *inviteplayer(const char *tty)
{
	char name[22];
	int fd;
	int n = strlen(this_tty);
	getfname(name, tty);
	fd = open(name, O_WRONLY | O_APPEND);
	if (fd < 0)
		return errmsg(name);
	this_tty[n] = '\n';
	while (write(fd, this_tty, n+1) == -1 && errno==EINTR)
		;
	close(fd);
	this_tty[n] = '\0';
	return NULL;
}

static char *errmsg_connect_local()
{
	char *msg;
	char *s = malloc(strlen(SUN_PATH(sock_addr))+9);
	strcpy(s, "connect ");
	strcat(s, SUN_PATH(sock_addr));
	msg = errmsg(s);
	free(s);
	return msg;
}

char *mksocket_local(const char *tty, int server)
{
	char *msg = mksocket(PF_UNIX);
	int size;
	if (!msg) {
		getaddr(tty, server);
		size = offsetof(struct sockaddr_un, sun_path)
				  + strlen(SUN_PATH(sock_addr));
		if (server) {
			unlink(SUN_PATH(sock_addr));
			msg = bind_listen(size);
			chmod(SUN_PATH(sock_addr), S_IREAD | S_IWRITE |
			       S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
			if (!msg && tty)
				msg = inviteplayer(tty);
		} else if (!connectsock(size))
			msg = errmsg_connect_local();
		if (msg)
			rmsocket();
	}
	return msg;
}

void connect_tty(const char *tty)
{
	char *msg, *p;
	if (!strncmp(tty, "/dev/", 5))
		tty += 5;
	printf("Connecting to %s...\n", tty);
	msg = mksocket_local(tty, 0);
	if (!msg)
		return;
	p = strchr(msg, '/');
	puts(p);
	free(msg);
	if (!get_this_tty())
		exit(1);
	if (!strncmp(tty, this_tty, 7)) {
		printf("%s is this terminal!\n", tty);
		exit(1);
	}
	puts("Failed.");
	printf("Sending invitation to %s...\n", tty);
	mkinvitfile();
	if (msg = mksocket_local(tty, 1)) {
		puts(msg);
		free(msg);
		exit(1);
	}
}

char *get_socket_fname()
{
	if (sock_addr)
		return SUN_PATH(sock_addr);
	return NULL;
}
