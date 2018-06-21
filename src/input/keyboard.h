extern int kb_no_autorep;

void init_keybd();

/* reads a key press into dest, returns number of bytes read */
int kb_readkey(unsigned char *dest);

int kb_getpress(int flags);

int kb_toascii(const unsigned char *key);

/* reset autorepeat timer for softdrop */
void kb_reset_drop(int pl);

/* throw away typeahead */
void kb_flushinp();

void kb_setmapping(const unsigned char *input, int keypress);

/* returns number of chars copied into dest */
int kb_getkeyfor(int keypress, unsigned char *dest, int fallback);

int kb_getchrfor(int keypress);

void kb_rmmapping(int keypress);

/* returns key name or null */
const char *kb_keyname(unsigned char *key, int n);
