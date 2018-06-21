#include <stdlib.h>	/* exit */
#include <string.h>
#include "menu.h"
#include "../input/input.h"
#include "../textgfx/textgfx.h"
#include "../draw/draw.h"

static int getdropdownwidth(const char **items, int n)
{
	int w = strlen(items[0]);
	int i = 1;
	while (i < n) {
		if (w < strlen(items[i]))
			w = strlen(items[i]);
		i++;
	}
	return w;
}

static void printdropdownitem(const char *name, int sel, int w)
{
	if (sel) {
		while (name[0]==' ' && name[1]==' ') {
			putch(' ');
			name++;
			w--;
		}
		setattr_normal();
	}
	printstr(name);
	if (strlen(name) < w)
		putch(' ');
	if (sel)
		setattr_standout();
}

int dropdownlist(const char **items, int n, int i, int x, int y)
{
	int w = getdropdownwidth(items, n);
	int j;
	while (is_outside_screen(0, y+n+1))
		y--;
	setattr_standout();
	drawbox(x, y, w+2, n+2, NULL);
	x++;
	y++;
	setcurs(x, y);
	for (j = 0; j < n; j++) {
		printdropdownitem(items[j], i==j, w);
		newln(x);
	}
	while (1) {
		setcurs(x-1, y+i);
		refreshwin(-1);
		switch (getkeypress_block(SINGLE_PL) & 0xFF) {
		case STARTBTN:
		case A_BTN:
			goto out;
		case 'q':
			exit(0);
		case MVUP:
			if (!i)
				continue;
			j = -1;
			break;
		case '\t':
			if (i == n-1) {
				j = -i;
				break;
			}
		case MVDOWN:
			if (i < n-1) {
				j = 1;
				break;
			}
		case MVRIGHT:
			continue;
		default:
			i = -1;
			goto out;
		}
		setcurs(x, y+i);
		printdropdownitem(items[i], 0, w);
		i += j;
		setcurs(x, y+i);
		printdropdownitem(items[i], 1, w);
	}
out:	clearbox(x-1, y-1, w+2, n+2);
	return i+1;
}

int selectitem(const char **items, int n, int *i, int k)
{
	int x, y;
	switch (k) {
	case 0:
		break;
	case MVLEFT:
		if (*i)
			*i -= 1;
		break;
	case MVRIGHT:
		if (*i < n-1)
			*i += 1;
		break;
	case STARTBTN:
	case A_BTN:
		get_xy(&x, &y);
		n = dropdownlist(items, n, *i, x, y);
		if (n)
			*i = n-1;
		setcurs(x, y);
		return 3;
	default:
		return 0;
	}
	putch('[');
	printstr(items[*i]);
	n = getdropdownwidth(items, n) - strlen(items[*i]);
	putnchars(' ', n);
	printstr("] ");
	return 1;
}
