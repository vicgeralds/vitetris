struct option {
	union val {
		int integ;
		char str[4];
		const void *p;
	} val;
	struct option *next;
	char tp_key[4];
};

#define opt_key(o) ((o)->tp_key+1)
#define opt_isint(o) ((o)->tp_key[0] == 0)
#define opt_isstr(o) ((o)->tp_key[0] == 1)

extern struct sect {
	char name[8];
	struct option *opts;
	struct sect *next;
} sect_hd;

/* returns 0 if converted to int, 1 if copied as string.
 * If value is a string of length >= 4, remove trailing space from str,
 * set val->p to pos in str, return 2. */
int strtoval(char *str, union val *val);

struct sect *addsect(const char *name);
void addopt(const char *key, union val val, int tp, struct sect *sect);

struct option *getoptions(const char *sect_name);
int getopt_int(const char *sect_name, const char *key);
char *getopt_str(const char *sect_name, const char *key);
char *opt_longstr(struct option *o);

/* type should be 0 if val is int, 1 if string, 2 if long string (val.p) */
void setoption(const char *sect_name, const char *key, union val, int type);

void unsetoption(const char *sect_name, const char *key);
void freeoptions(const char *sect_name);
