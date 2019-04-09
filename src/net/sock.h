/* socket flags */
#define IS_SERVER 1
#define CONNECTED 2
#define CONN_BROKEN 4
#define PL2_IN_GAME 8
#define WAIT_PL2INGAME 16
#define SAME_HEIGHT 32
#define PL2_GARBAGE 64
#define CONN_PROXY 128
#define PLIST_UNHANDLED 256

#ifndef SOCKET_EMPTY_DEFS
extern int sock_flags;
#else
#define sock_flags 0
#endif
#define is_server (sock_flags & IS_SERVER)

extern char my_name[17];
extern char opponent_name[17];

extern struct player_id {
	unsigned char id;
	char name[17];
	char mode[4];
} *playerlist;
extern int playerlist_n;

extern
const struct invit {
	char user[16];
	char tty[8];
} *invit;

extern char this_tty[8];

void mkinvitfile();
int checkinvit();
void rminvitfile();

/* ttys should be of size 8*n - n strings of length 8 */
int get_2p_ttys(char *ttys, int n);

/* tty is opponent's tty (may be NULL if server).
 * returns malloc'd error message on error, otherwise NULL */
char *mksocket_local(const char *tty, int server);

/* makes server socket if name is NULL.
 * prints error message and returns 0 on failure */
int mksocket_inet(const char *name, unsigned port);

int reconnect_inet();
int is_inet();

/* exits with error message on failure */
void connect_tty(const char *tty);

/* returns the address of the listening socket or NULL */
char *get_socket_fname();

void rmsocket();

/* Game protocol functions */

void request_playerlist();
void connect_to_player(struct player_id *p);
int reconnect_server();

#ifndef SOCKET_EMPTY_DEFS
void sock_sendbyte(char byte);

void sock_initgame();
void sock_sendplayer();
int sock_wait_pl2ingame();
void sock_initround();

#ifdef tetris_h
void sock_sendnext(const struct player *p, char n);
void sock_sendpiece(const struct player *p);
#endif
void sock_sendgarbage_num(int n);
void sock_sendwinner();

#else /* if SOCKET_EMPTY_DEFS */
#define sock_sendbyte(b)
#define sock_wait_pl2ingame() 0
#define sock_initround()
#define sock_sendpiece(p)
#define sock_sendgarbage_num(n)
#define sock_sendwinner()
#endif

int sock_getkeypress(int flags);

void conn_broken(int sig);
void accept_conn();
void writebytes(const char *buf, int n);
int readbytes(char *buf, int n);
int waitinput_sock(unsigned msec);
