#undef TWOPLAYER
#define TWOPLAYER 1
#include "menu.h"
#include "internal.h"
#include "../input/input.h"
#include "../textgfx/textgfx.h"
#include "../draw/draw.h"

int gamemenu_2p(const char **menu, int x, int y, menuhandler *handlers)
{
	int yy = y;
	int keypr;
	int i1 = 0;
	int i2 = 0;
	int *i = &i1;
begin:	draw_2p_menu_decor(1, x-1, y);
	draw_2p_menu_decor(2, x-1, y+GAMEMENU_LENGTH);
	x += 2;
back:	player_ = 1;
	drawmenu(menu, GAMEMENU_LENGTH-1, i1, x, y, handlers);
	player_ = 2;
	drawmenu(menu, GAMEMENU_LENGTH-1, i2, x, y+GAMEMENU_LENGTH, handlers);
	while (1) {
		setcurs(x, yy+*i);
		refreshwin(-1);
		keypr = getkeypress_block(0);
		if (keypr==STARTBTN && i1!=3 && i2==3)
			keypr |= PLAYER_2;
		else if (inputdevs_player[0]==2 && i1==3 && i2!=3 &&
						keypr==(STARTBTN | PLAYER_2))
			keypr = STARTBTN;
		if (!(keypr & PLAYER_2)) {
			player_ = 1;
			i = &i1;
			yy = y;
		} else {
			player_ = 2;
			i = &i2;
			yy = y+GAMEMENU_LENGTH;
		}
		setcurs(x, yy+*i);
		switch (handle_menuitem(menu, GAMEMENU_LENGTH-1, i,
					x, yy+*i, handlers, keypr)) {
		case 0:
			return 0;
		case 2:
			if (*i==3) {
				x -= 2;
				clearbox(0, y+9, 0, 4);
				inputsetup_box(player_, x, y);
				*i = 0;
				goto begin;
			}
			return 1;
		case 3:
			goto back;
		}
	}
}
