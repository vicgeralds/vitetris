#ifdef NCURSES_VERSION
#define COLOR_1_6(i) (i+1)
#define COLOR_DEFAULT_BG -1
#else
#define COLOR_1_6(i) colors1_6[i]
#define COLOR_DEFAULT_BG COLOR_BLACK
#endif

extern const short colors1_6[6];

extern int margin_x;

extern WINDOW *window;
extern WINDOW *wins[6];

void init_color_pairs();
void initpair(short pair, short f, short b);

void delwins();
