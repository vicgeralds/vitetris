/* returns 0 if key is unrecognized, -1 if val is invalid */
int testgameopt(const char *key, int val, int pl);

#ifndef TWOPLAYER
#define allocgame(twop) struct game_1p gm1p; game = (struct game *) &gm1p
#else
#define allocgame(twop) struct game_2p gm2p; game = (struct game *) &gm2p
#endif

void initgame();

/* returns 1 to play again */
int startgame();

const char *get_wonlost_stats(const char *me, const char *opponent);
void	    upd_wonlost_stats(const char *me, const char *opponent, int won);
