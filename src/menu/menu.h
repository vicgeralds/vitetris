#ifndef menu_h
#define menu_h

/* menuhandler parameters:
 * keypress  lower 8 bits of getkeypress return value
 * pos       pointer to menu item index number >= 0
 *
 * Return values:
 *   0    unhandled keypress (leave to default handler)
 *   ESC  go back
 *   1    stay in menu
 *   2    proceed
 *   3    redraw menu
 */
typedef int (*menuhandler)(int keypress, int *pos);

/* Print a menu item, selected if sel.  Leaves the cursor after name. */
void printmenuitem(const char *name, int sel);

/* Print space-separated words.
 * sel_index is used to mark one word as selected (counting from zero). */
void printmenuitem_options(const char *str, int sel_index);

/* Draw menu with n items and pos as selected index number.
 * handlers may be NULL or an array with some null pointers. */
void drawmenu(const char **menu, int n, int pos, int x, int y,
	      menuhandler *handlers);

/* Prints menu item(s) and calls menu handlers if given.
 * i is a pointer to update with new selected menu-item index.
 * x and y is the position of the current item and where the cursor is.
 * keypr is the return value of getkeypress.
 * This function calls exit for 'q'.
 * Default handler returns:
 *   0  go back (B button)
 *   1  no action
 *   2  proceed (A button) */
int handle_menuitem(const char **menu, int n, int *i, int x, int y,
		    menuhandler *handlers, int keypr);

/* Returns 0 if a handler returned 0.
 * Returns selected index + 1 if a handler returned 2. */
int openmenu(const char **menu, int n, int i, int x, int y,
	     menuhandler *handlers);

/* Handles a menu with two columns (extends handle_menuitem).
 * h is the number of items in the first column.
 * Returns 3 if column changed. */
int handle_menuitem_2cols(const char **menu, int n, int *i, int h,
			  int x, int y, menuhandler *handlers, int keypr);

/* Prints an editable text field with the cursor at pos. */
void printtextbox(const char *text, int pos);

int rarrow_menuitem(int keypress, int *pos);

/* Same as openmenu but different appearance. */
int dropdownlist(const char **items, int n, int i, int x, int y);

/* The selected item is printed inside a box which may expand to a
 * dropdownlist.  Returns
 *   0  if keypress != 0 and was unhandled -- nothing printed,
 *   3  if a dropdownlist was opened (redraw),
 *   1  otherwise -- the selected item was printed. */
int selectitem(const char **items,
	       const char **abbr, int n, int *i, int keypress);

/* ___________
  |   MENUS   |
   ^^^^^^^^^^^   */
int startupmenu(int i);
int gamemenu();		/* opened when game mode is set */
int netplaymenu();	/* connect screen */
void hiscorelist();
void optionsmenu();
int gameovermenu();

#define GAMEMENU_LENGTH 5

/* inner functions */
int startup_menu(int i, int x, int y);
int game_menu(int i, int x, int y);
int netplay_menu(int x, int y);
void options_menu(const char **items, int n, menuhandler f, int x, int y);

void show_hiscorelist(int x, int y);
void show_hiscorelist5(int x, int y, int i);

int select_2p_tty(int x, int y);
int menu_checkinvit(int x, int y);

extern int player_;  /* 0 - single player
			1 - player1
			2 - player2 */

void inputsetup_screen(int player, int x, int y);

struct termopt {
	unsigned char flag;
	const char *s;
	const char *key;
};
int term_optionhandler(int k, const struct termopt *o);

int getblockstyle();
int select_blockstyle(int k);

const char *getmodestr();

#ifndef NULL
#define NULL ((void *) 0)
#endif

#endif /* !menu_h */
