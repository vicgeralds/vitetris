
#define DEFAULT_ROTATION (ROT_CLOCKWISE | ROT_LEFTHAND)

/* returns 0 if key is unrecognized, -1 if val is invalid */
int testgameopt(const char *key, int val, int pl);

void creategame();

/* returns 1 to play again */
int startgame();

const char *get_wonlost_stats(const char *me, const char *opponent);
void	    upd_wonlost_stats(const char *me, const char *opponent, int won);
