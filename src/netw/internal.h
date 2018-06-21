extern struct sockaddr *sock_addr;

const char *sock_strerror();
char *sock_errmsg(const char *descr);
#define errmsg(descr) sock_errmsg(descr)

char *mksocket(int domain);
char *bind_listen(int size);
int connectsock(int size);

#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#endif

#ifndef S_IREAD
#define S_IREAD S_IRUSR
#endif
#ifndef S_IWRITE
#define S_IWRITE S_IWUSR
#endif
#ifndef O_NONBLOCK
#define O_NONBLOCK O_NDELAY
#endif
#ifndef EAGAIN
#define EAGAIN EWOULDBLOCK
#endif

#ifndef offsetof
#define offsetof(type, member) ((size_t) ((type *) 0)->member)
#endif

#undef SUN_PATH
#define SUN_PATH(ptr) (((struct sockaddr_un *) (ptr))->sun_path)
