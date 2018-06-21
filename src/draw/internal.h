extern char board_bottom_color[2];

void drawbl(int bl, int clr, int x, int y);
void drawstr(const char *str, int n, int x, int y);
void drawboard(int pl);
void drawpanel_labels(const char *first, int x);
void drawpanel_bordercolor(int clr);
int draw_vline(int x, int y, int h);

#ifdef TWOPLAYER
void printstat_2p(const struct player *p);
#else
#define printstat_2p(p)
#endif
