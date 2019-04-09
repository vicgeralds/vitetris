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

/* delays in millisecs */
#define SPAWN_DELAY 166
#define LOCK_DELAY  166
#define CLEAR_DELAY 332

/* max number of moves/rotations during lock delay */
#define LOCK_DELAY_MOVES 10

struct tetr {
	short blocks;
	signed char x, y;
	char color;
	char state;	/* rotation state 0-3 */
};

struct tetr_pos {
	short blocks;
	signed char x, y;
};

struct lockdelay {
	int num_moves;
	struct tetr_pos moves[LOCK_DELAY_MOVES];
};

/* rotation flags */
#define ROT_CLOCKWISE 1
#define ROT_LEFTHAND  2
#define ROT_MODERN    4

struct player {
	char startlevel;
	char height;
	char lineslimit;
	char rotation;
	struct tetr piece;
	struct tetr *next;
	uint_least32_t board[20];
	int_least32_t score;
	signed char level;
	signed char initrot;	/* initial rotation */
	char  floorkick;
	short falltime;
	short lines;
	short mvleft_tm;
	short mvright_tm;
	struct lockdelay lockdelay;
};

/* game states */
#define GAME_NULL    0
#define GAME_CREATED 1
#define GAME_RUNNING 2
#define GAME_PAUSED  3
#define GAME_OVER    4

/* game modes */
#define MODE_1P 1
#define MODE_2P 2
#define MODE_B  4
#define MODE_NET 8
#define MODE_40L 16

#define MODE_1P_B	(MODE_1P | MODE_B)
#define MODE_1P_40L	(MODE_1P | MODE_40L)
#define MODE_LINECLEAR	(MODE_B  | MODE_40L)

struct game {
	int state;
	int mode;
	struct player player[2];
	char clearedlines[4];
	unsigned char data[8];
};

extern struct game game;

#define player1 	game.player[0]
#define player2 	game.player[1]
#define tetr_stats	game.data
#define softdrop_speed	game.data[7]

#define TWOPLAYER_MODE (game.mode & MODE_2P)
#define isplayer2(p)   ((p) > &player1)

int randnum(int n);
int rand_tetrom_next();
void gettetrom(struct tetr *t, int i, int rot);

int hitbtm(struct tetr *piece, struct player *p);
int test_lockdelay_move(struct player *p);
void lockpiece(struct player *p);
void moveright(struct player *p);
void moveleft(struct player *p);
int  movedown(struct player *p, int drop);
void rotate  (struct player *p, int clockwise);
int  harddrop(struct player *p, int safe);
int  softdrop(int n, int safe);

void initialrotate(struct player *p, int clockwise);
int  spawnpiece(struct player *p);

void setupplayer(struct player *p);
int startgame_1p();
int startgame_wait(int flags);
int pausegame();

#endif /* !tetris_h */
