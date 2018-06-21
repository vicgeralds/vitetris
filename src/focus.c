#include "focus.h"
#if ALLEGRO
#include "textgfx/textgfx.h"

int in_xterm = 1;

int xterm_hasfocus()
{
	if (textgfx_flags & LOST_FOCUS)
		return 0;
	return 1;
}

#elif XLIB
#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>

int in_xterm = 0;

static Display *dpy;
static Window xtermwin;

void in_xterm_init()
{
	char *s = getenv("WINDOWID");
	if (s && sscanf(s, "%lu", &xtermwin)==1 && (dpy = XOpenDisplay(0)))
		in_xterm = xterm_hasfocus();
}

int xterm_hasfocus()
{
	Window w, focusw, root, par, *children;
	unsigned int n;
	int i;
	if (!XGetInputFocus(dpy, &w, &i))
		return 0;
	if (w == xtermwin)
		return 1;
	focusw = w;
	w = xtermwin;
	while (XQueryTree(dpy, w, &root, &par, &children, &n) && par != root) {
		if (children) {
			for (i = 0; i < n; i++)
				if (children[i] == focusw) {
					focusw = xtermwin;
					break;
				}	
			XFree(children);
			if (focusw == xtermwin)
				return 1;
		}
		if (par == focusw)
			return 1;
		w = par;
	}
	return 0;
}
#endif /* XLIB */
