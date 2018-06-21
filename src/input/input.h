/* keypress values */
#define MVLEFT 1
#define MVRIGHT 2
#define MVUP 3		/* rotate */
#define MVDOWN 4	/* softdrop */
#define A_BTN 5		/* rotate clockwise */
#define B_BTN 6		/* rotate anticlockwise */
#define HARDDROP ' '
#define STARTBTN '\n'
#define ESC '\033'
#define DEL 0x7F
#define BACKSPACE '\b'

/* keypress bit flags */
#define IN_GAME 0x100
#define PLAYER_1 0x200
#define PLAYER_2 0x400

#define SINGLE_PL 0x800

#define DAS_INITIAL_DELAY 266
#define DAS_DELAY 100

extern const char input_chr9[9][4];

#if SOCKET
#define NUM_INPUTDEVS 4
#elif JOYSTICK
#define NUM_INPUTDEVS 3
#else
#define NUM_INPUTDEVS 1
#endif

#define socket_fd inputdevs_fd[3]

extern int inputdevs_fd[4];
extern char inputdevs_player[4];

extern int num_joyst;
extern int autorep_a_b_btns;
extern int edit_mode;

void init_inputdevs();

/* prepare for two-player game */
void initplayerinput();

/* returns NUM_INPUTDEV if no input is ready */
int inpselect_dev(int tm);

/* wait msec < 1000 milliseconds for input on file descriptor */
int waitinput(int fd, unsigned msec);

int getkeypress(int tm, int flags);
int getkeypress_block(int flags);

void spawn_discard_drops(int pl);

int setkeymapping(int dev, int keypress);
const char *getkeyfor_str(int dev, int keypress);

/* processkey_ingame flags */
#define DISCARD_MOVES 1
#define DISCARD_DROPS 2
#define NO_PAUSE 8

int processkey_ingame(int keypress, int flags);
