#include <stdlib.h>
#include <string.h>
#include "menu.h"
#include "menuext.h"
#include "../input/input.h"
#include "../textgfx/textgfx.h"
#include "../draw/draw.h"
#undef TTY_SOCKET
#define TTY_SOCKET 1
#include "../netw/sock.h"

int yesno_menu(int x, int y);

static int invit_dialog(const char *user, const char *tty, int x, int y)
{
	char word[7];
	const char *p = "wants to battle you. ";
	const char *q;
	int i, n;
	drawbox(x, y, 32, 7, "2P GAME INVITATION");
	setcurs(x+2, y+2);
	printstr(user);
	printstr(" on ");
	printstr(tty);
	i = strlen(user) + strlen(tty) + 4;
	while (*p) {
		q = strchr(p, ' ');
		n = q-p;
		if (i+n > 27) {
			newln(x+2);
			i = 0;
		} else {
			putch(' ');
			i++;
		}
		strncpy(word, p, n);
		word[n] = '\0';
		printstr(word);
		i += n;
		p = q+1;
	}
	setcurs(x+11, y+5);
	printstr("Accept?  ");
	i = yesno_menu(x+20, y+5);
	clearbox(x, y, 32, 7);
	return i;
}

int select_2p_tty(int x, int y)
{
	const char *items[4] = {"this terminal"};
	char ttys[3][8];
	char *msg;
	int i;
	if (!get_2p_ttys(ttys[0], 3))
		return 1;
	for (i = 0; i < 3 && *ttys[i]; i++)
		items[i+1] = ttys[i];
	i = dropdownlist(items, i+1, 0, x, y);
	if (!i)
		return 0;
	if (i > 1 && (msg = mksocket_local(ttys[i-2], 1))) {
		setcurs_end();
		printstr(msg);
		free(msg);
		return 0;
	}
	return 1;
}

int menu_checkinvit(int x, int y)
{
	const struct invit *inv;
	int accpt;
	char *msg;
	if (!invit)
		return 0;
	while (invit) {
		inv = invit;
		invit = NULL;
		accpt = invit_dialog(inv->user, inv->tty, x, y);
		msg = mksocket_local(inv->tty, 0);
		if (accpt) {
			if (!msg) {
				rminvitfile();
				return 2;
			}
			setcurs_end();
			printstr(msg);
			cleartoeol();
		} else
			rmsocket();
		if (msg)
			free(msg);
	}
	return -1;
}
