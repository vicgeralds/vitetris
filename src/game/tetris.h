#ifndef tetris_h
#define tetris_h

#include "int32t.h"

#define TETR_I  0xF00	/* |    | #  | */
#define TETR_I2 0x2222	/* |    | #  | */
			/* |####| #  |
			   |    | #  | */

#define TETR_J  0x470	/* |    | #  |#   | ## | */
#define TETR_J2 0x322	/* |#x# | x  |#x# | x  | */
#define TETR_J3 0x71	/* |  # |##  |    | #  | */
#define TETR_J4 0x226

#define TETR_L  0x170	/* |    |##  |  # | #  | */
#define TETR_L2 0x223	/* |#x# | x  |#x# | x  | */
#define TETR_L3 0x74	/* |#   | #  |    | ## | */
#define TETR_L4 0x622

#define TETR_O  0x66	/* | ## | */
			/* | ## | */

#define TETR_S  0x360	/* |    |#   | */
#define TETR_S2 0x231	/* | x# |#x  | */
			/* |##  | #  | */

#define TETR_T  0x270	/* |    | #  | #  | #  | */
#define TETR_T2 0x232	/* |#x# |#x  |#x# | x# | */
#define TETR_T3 0x72	/* | #  | #  |    | #  | */
#define TETR_T4 0x262

#define TETR_Z  0x630	/* |    | #  | */
#define TETR_Z2 0x132	/* |#x  |#x  | */
			/* | ## |#   | */

#define SPAWN_DELAY 166
#define LOCK_DELAY 166
#define CLEAR_DELAY 332

struct tetr {
	short blocks;
	signed char x, y;
	char color;
};

/* rotation sys flags */
#define ROT_CLOCKWISE 1
#define ROT_LEFTHAND 2

struct player {
	char startlevel;
	char height;
	char lineslimit;
	char rotationsys;
	struct tetr piece;
	uint_least32_t board[20];
	int_least32_t score;
	signed char level;
	short falltime;
	short lines;
	short mvleft_tm;
	short mvright_tm;
};

/* game modes */
#define MODE_1PLAYER 1
#define MODE_2PLAYER 2
#define MODE_BTYPE 4
#define MODE_NETWORK 8

/* game states */
#define GAME_RUNNING 1
#define GAME_PAUSED 2
#define GAME_OVER 4

#define GAME_members(n) 	\
	char mode;		\
	char state;		\
	struct tetr *next;	\
	struct player player[n]	\

extern struct game {
	GAME_members(1);
} *game;

struct game_1p {
	GAME_members(1);
	unsigned char data[8];
};

struct game_2p {
	GAME_members(2);
};

#define game_running  (game->state & GAME_RUNNING)
#define game_paused   (game->state & GAME_PAUSED)
#define game_over     (game->state & GAME_OVER)

#define player1  game->player[0]
#define player2  game->player[1]

#define tetr_stats	((struct game_1p *) game)->data
#define softdrop_speed	((struct game_1p *) game)->data[7]

extern char clearedlines[4];

int randnum(int n);
void gettetrom(struct tetr *t, int i);

int hitbtm(struct tetr *piece, struct player *p);
void lockpiece(struct player *p);
void moveright(struct player *p);
void moveleft(struct player *p);
int movedown(struct player *p, int drop);
void rotate(struct player *p, int clockwise);
int harddrop(struct player *p, int safe);
int softdrop(int n, int safe);

void setupplayer(struct player *p);
int startgame_1p();
int startgame_wait(int flags);
int pausegame();

#ifdef TWOPLAYER
#define TWOPLAYER_MODE (game->mode & MODE_2PLAYER)
#define isplayer2(p) ((p) > game->player)
#else
#define TWOPLAYER_MODE 0
#define isplayer2(p) 0
#endif

#endif /* !tetris_h */
