extern char curs_x;
extern char curs_y;
extern char margin_x;
extern char menuheight;

extern int win_x;
extern int win_y;

void set_ansi_color(int bg, int fg, char bold);

#include "config.h"
#if !UNIX || __CYGWIN__
#define IBMGRAPHICS 1
#endif
int ibmgfx(int ch);
