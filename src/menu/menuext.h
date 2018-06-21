int startup_menu(int i, int x, int y);

int select_2p_tty(int x, int y);
int menu_checkinvit(int x, int y);

int netplay_menu(int x, int y);

#ifdef menu_h
void options_menu(const char **items, int n, menuhandler f, int x, int y);
#endif

struct termopt {
	unsigned char flag;
	const char *s;
	const char *key;
};
int term_optionhandler(int k, const struct termopt *o);

int getblockstyle();
int select_blockstyle(int k);

/* Outer functions */
int startupmenu(int i);
int gamemenu();
int netplaymenu();
void hiscorelist();
void optionsmenu();
