#include "draw.h"
#include "internal.h"
#include "../textgfx/textgfx.h"
#include "../version.h"

void draw_tetris_logo(int x, int y)
{
	int bl;
	drawbl(0x227, 1, x, y);
	drawbl(0x313, 4, x+7, y);
	drawbl(1, 4, x+8, y+1);
	drawbl(0x227, 6, x+12, y);
	drawbl(0x113, 7, x+19, y);
	drawbl(0x111, 2, x+24, y);
	bl = 0x326;
	if (is_outside_screen(x+33, 0))
		bl = 0x322;
	drawbl(bl, 3, x+27, y);
	setattr_normal();
}

void print_vitetris_ver(int x, int y)
{
	setcurs(x, y);
	setattr_bold();
	printstr(VITETRIS_VER);
	setattr_normal();
}
