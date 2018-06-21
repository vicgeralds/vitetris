#include "draw.h"
#include "internal.h"
#include "../textgfx/textgfx.h"

void draw_2p_menu_decor(int pl, int x, int y)
{
	x++;
	drawstr("\\l\\x", 0, x, y-1);
	y += 2;
	setcurs(x-1, y);
	putch(pl+'0');
	drawstr("P\\x\\2m", 0, x, y);
}
