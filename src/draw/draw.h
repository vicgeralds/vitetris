extern char tetrom_colors[7];

void draw_tetris_logo(int x, int y);
void print_vitetris_ver(int x, int y);

#ifdef TWOPLAYER
int board_x(int pl, int col);
#else
#define board_x(pl, col) (2*col+11)
#endif
void next_xy(int pl, int *x, int *y);

void drawgamescreen_1p();
void drawgamescreen_2p();
void print_game_message(int pl, const char *str, int bold);
void print_press_key();

/* tetris.h should be included before */
#ifdef tetris_h

void drawnext(const struct player *p, const struct tetr *next);

enum drawmode {
	DRAW_PIECE,
	DRAW_BLOCKS,
	CLEAR_BLOCKS
};

/* x and y here means column and row in the player's playfield */

void drawblocks(const struct player *p, int bl, int x, int y,
		enum drawmode mode);
#define drawpiece(plr, bl, x, y)   drawblocks(plr, bl, x, y, DRAW_PIECE)
#define clearblocks(plr, bl, x, y) drawblocks(plr, bl, x, y, CLEAR_BLOCKS)

void drawrow(const struct player *p, int y);

/* redraw row y and all rows above */
void redrawboard(const struct player *p, int y);

void clearboard_paused();
void upd_stat(const struct player *p, int levelup);
void upd_garbagemeter(const struct player *p, int n);
void show_winner(const struct player *p);

void upd_dropmarker(const struct player *p, int mv);
void hide_dropmarker(const struct player *p);
#define show_dropmarker(p) upd_dropmarker(p, 0)

#endif /* tetris_h */

void drawbox(int x, int y, int w, int h, const char *title);
void clearbox(int x, int y, int w, int h);

void draw_2p_menu_decor(int pl, int x, int y);

void upd_screen(int win);
