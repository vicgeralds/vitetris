#include "../config.h"

#if UNIX && !CURSES && !ALLEGRO
#define ESCSEQ 1
#endif

int readchr();
int readescseq(char *s);
int transl_escseq(const char *s, int flags);
const char *getescseq_str(int keypr);
void mapescseq(const char *str, int keypr);
void rmescseq(int keypress);
void free_escape_sequences();
