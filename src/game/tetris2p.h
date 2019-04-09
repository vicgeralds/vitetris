#ifdef SOCKET
#define NETPLAY (game.mode & MODE_NET)
#else
#define NETPLAY 0
#endif

extern struct tetris2p {
	int falltm;
	int delay;
	short lockdelay;	/* boolean */
	short x;
	char clearedlines[5];	/* clearedlines[4] = null byte */
	signed char garbage[3];
} tetris2p[2];

extern struct player *winner;
extern char *tetrom_seq;

void getgarbage_player2();
int nextpiece_2p(struct player *plr);
int startgame_2p();
