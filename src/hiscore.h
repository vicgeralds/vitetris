extern int hiscores_read;
extern int num_hiscores;
extern const char *hiscore_columns;

/* player1.score > hiscores[9].score */
int ishiscore();
/* player1.score > hiscores[i].score */
int isbetterscore(int i);	

void readhiscores(const char *filename);
void sethiscorecontext();

/* returns 0 if highscore could not be saved */
int savehiscore(const char *name);

/* returns hiscores[i].name or buf */
const char *gethiscorename(int i, char *buf);

/* get formatted score in buf */
void gethiscorestr(int i, char *buf);

/* Get formatted hiscorelist in buf, which should be of size 320.
 * every row is 32 chars
 * returns number of rows */
int gethiscorelist(char *buf);
