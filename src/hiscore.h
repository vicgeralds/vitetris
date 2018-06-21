#include "int32t.h"

extern
struct hiscore {
	char name[8];
	int_least32_t score;
	char startlevel;
	char level;
	short lines;
} hiscores[10];

#define ishiscore() (player1.score >= 12000 && \
		     player1.score > hiscores[9].score)

int readhiscores(const char *filename);

/* returns 0 if highscore could not be saved */
int savehiscore(const char *name);

/* returns hiscores[i].name or buf */
const char *gethiscorename(int i, char *buf);

/* Get formatted hiscorelist in buf, which should be of size 320.
 * every row is 32 chars
 * returns number of rows */
int gethiscorelist(char *buf);
