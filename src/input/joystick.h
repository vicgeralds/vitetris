/* true until we receive any events */
extern int js_dead;

/* prints error message if open fails */
int js_open(int i, const char *devname);

void init_joysticks();

/* reads a button press */
int js_readbtn(int i);

/* get translated keypress */
int js_getpress(int i, int flags);

int getautorepeat(int flags);
int test_autorep_tm(short *tm);

/* set autorepeat timer for btn (not axis) */
void js_pressbtn(int i, int btn);
void js_releasebtn(int i, int btn);

int js_pressed(int i);

/* returns button press to repeat */
int js_autorep(int i, short **tm_ret);

/* reset autorepeat timer for softdrop */
void js_reset_drop(int i);

void js_setmapping(int i, int btn, int keypress);
int js_getbtnfor(int i, int keypress);
void js_rmmapping(int i, int keypress);

/* set mapping if not set */
void js_setifnull(int i, int btn, int keypress);

/* set default mappings if null */
void js_default_buttons(int i);

const char *js_btnname(int btn);
