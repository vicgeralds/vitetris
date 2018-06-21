#include "textgfx.h"
#include "../draw/draw.h"
#include "../game/tetris.h"

#if !NO_MENU
int in_menu = 1;
#endif

int getmargin_x()
{
	int x;
	if (term_width <= 32)
		x = 0;
	else if (!in_menu && TWOPLAYER_MODE) {
		if (term_width < 58)
			x = 0;
		else
			x = (term_width-56)/2;
	}
	else if (term_width < 48)
		x = 1;
	else
		x = (term_width-(in_menu ? 34 : 44))/2;
	return x;
}

void getwin_xy(int win, int *x, int *y)
{
	*y = 0;
	switch (win) {
	case 0:
		*x = 0;
		return;
	case 1:
	case 2:
		*x = board_x(win, 0);
		break;
	case WIN_NEXT:
	case WIN_NEXT+1:
		next_xy(1+win-WIN_NEXT, x, y);
		return;
	case WIN_PANEL:
		*x = TWOPLAYER_MODE ? 22 : 0;
		break;
	case WIN_TETROM_STATS:
		*y = 11;
	case WIN_TOP_SCORES:
		*x = board_x(1, 12);
		if (!*y && !_HEIGHT_24L)
			*y = 1;
	}
	if (_HEIGHT_24L)
		*y += 4;
}
