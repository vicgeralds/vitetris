#include <stdlib.h>	/* exit */
#include <string.h>
#include <ctype.h>
#include "menu.h"
#include "../input/input.h"
#include "../textgfx/textgfx.h"

void printmenuitem(const char *name, int sel)
{
#if !NO_MENU
	if (*name == '-') {
		putch(' ');
		if (!name[1])
			putnchars('-', 16);
		else
			printstr(name);
		return;
	}
#endif
	if (!sel)
		setcolorpair(MAGENTA_FG);
	else if (!_MONOCHROME)
		setcolorpair(WHITE_ON_BLUE);
	else
		setattr_standout();
	if (sel && (textgfx_flags & TT_MONO)==TT_BLOCKS) {
		while (*name==' ') {
			putch(' ');
			name++;
		}
		putch('*');
	} else
		putch(' ');
	printstr(name);
	putch(' ');
	setattr_normal();
	movefwd(1);
}

void printmenuitem_options(const char *str, int sel)
{
	char buf[16];
	const char *p = str;
	int ret;
	while (1) {
		if (p = strchr(str, ' ')) {
			memcpy(buf, str, p-str);
			buf[p-str] = '\0';
		} else
			strcpy(buf, str);
		if (!sel) {
			setattr_standout();
			ret = printstr(buf);
			setattr_normal();
		} else
			ret = printstr(buf);
		if (!ret || !p)
			break;
		putch(' ');
		str = p+1;
		sel--;
	}
}

void drawmenu(const char **menu, int n, int pos, int x, int y,
	      menuhandler *handlers)
{
	int i;
	setcurs(x, y);
	for (i = 0; i < n; i++) {
		printmenuitem(menu[i], (i==pos));
		if (handlers && handlers[i])
			handlers[i](0, &i);
		newln(x);
	}
}

#ifndef NO_MENU
static int find_firstletter(const char **menu, int n, int i, int c)
{
	int j = 1;
	c = toupper(c);
	while (i+j < n) {
		if (c == menu[i+j][0])
			return j;
		j++;
	}
	for (j=0; j < i; j++)
		if (c == menu[j][0])
			return j-i;
	return 0;
}
#endif

int handle_menuitem(const char **menu, int n, int *i, int x, int y,
		    menuhandler *handlers, int keypr)
{
	int j = *i;
	int m;
	keypr &= 0xFF;
	if (keypr == ESC)
		return 0;
	printmenuitem(menu[j], 1);
	if (handlers && handlers[j])
		switch (m = handlers[j](keypr, i)) {
		case 0:   break;
		case ESC: return 0;
		default:  return m;
		}
	switch (keypr) {
	case STARTBTN:
	case A_BTN:
		return 2;
	case B_BTN:
	case '\b':
		return 0;
	case 'q':
		exit(0);
	case MVUP:
		if (!j)
			return 1;
#if NO_MENU
		m = -1;
#else
		m = -1-(*menu[j-1]=='-');
#endif
		break;
	case '\t':
		if (j == n-1) {
			m = -j;
			break;
		}
	case MVDOWN:
		if (j < n-1) {
#if NO_MENU
			m = 1;
#else
			m = 1+(*menu[j+1]=='-');
#endif
			break;
		}
	default:
#ifndef NO_MENU
		if (m = find_firstletter(menu, n, j, keypr))
			break;
#endif
		return 1;
	}
	setcurs(x, y);
	printmenuitem(menu[j], 0);
	setcurs(x, y+m);
	printmenuitem(menu[*i+=m], 1);
	return 1;
}

int openmenu(const char **menu, int n, int i, int x, int y,
	     menuhandler *handlers)
{
	drawmenu(menu, n, i, x, y, handlers);
	while (1) {
		setcurs(x, y+i);
		refreshwin(-1);		/* refresh active window */

		switch (handle_menuitem(menu, n, &i, x, y+i, handlers,
					getkeypress_block(SINGLE_PL))) {
		case 0:
			return 0;
		case 2:
			return i+1;
#if !NO_MENU
		case 3:
			return openmenu(menu, n, i, x, y, handlers);
#endif
		}
	}
}

int handle_menuitem_2cols(const char **menu, int n, int *i, int h,
			  int x, int y, menuhandler *handlers, int keypr)
{
	menuhandler *p;
	int j = *i;
	int m;
	keypr &= 0xFF;
	switch (keypr) {
	case MVUP:
		if (j == h) {
			m = -1;
			goto col;
		}
		break;
	case '\t':
		if (j == n-1) {
			m = -j;
			goto col;
		}
	case MVDOWN:
		if (j == h-1) {
			m = 1;
			goto col;
		}
	}
	if (j < h) {
		if (keypr == MVRIGHT && j+h < n) {
			m = h;
			goto col;
		}
		m = handle_menuitem(menu, h, i, x, y, handlers, keypr);
	} else {
		if (keypr == MVLEFT && j-h >= 0) {
			m = -h;
			goto col;
		}
		j = *i-h;
		p = NULL;
		if (handlers)
			p = handlers+h;
		m = handle_menuitem(menu+h, n-h, &j, x, y, p, keypr);
		*i = j+h;
	}
	return m;
col:	printmenuitem(menu[j], 0);
	*i += m;
	return 3;
}

void printtextbox(const char *text, int pos)
{
	int underscore;
	if (_WHITE_BG)
		underscore = textgfx_flags & (ASCII | GNOME_TERM) || !_XTERM;
	else
		underscore = textgfx_flags & (MONOCHROME | TT_BLOCKS);
	while (*text) {
		setattr_normal();
		if (!pos)
			setattr_standout();
		else {
			if (_WHITE_BG) {
				if (!underscore || textgfx_flags & GNOME_TERM)
					setattr_underline();
			} else
				setcolorpair(WHITE_ON_BLUE);
		}
		if (*text == ' ' && underscore)
			putch('_');
		else
			putch(*text);
		text++;
		pos--;
	}
	setattr_normal();
}

#if !NO_MENU
int rarrow_menuitem(int k, int *p)
{
	printstr("\b->");
	return k==MVRIGHT ? 2 : 0;
}
#endif
