#ifndef NULL
#define NULL (void*)0
#endif

void inp_printkeys(int dev, int x, int y);
int inp_devlist(int *dev, int x, int y);
#ifdef JOYSTICK
int inputsetup_menuitem(int k, int *p);
#else
#define inputsetup_menuitem rarrow_menuitem
#endif

#ifdef TWOPLAYER
#define GAMEMENU_LENGTH 7
int gamemenu_2p(const char **menu, int x, int y, menuhandler *handlers);
#else
#define gamemenu_2p(menu, x, y, handlers) 0
#endif

#ifdef SOCKET
int gamemenu_socket(const char **menu, int i, int x, int y, menuhandler *hs);
#else
#define gamemenu_socket(menu, i, x, y, handlers) 0
#endif
