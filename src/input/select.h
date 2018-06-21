#include "../config.h"
#if HAVE_SYS_SELECT_H
#include <sys/select.h>
#elif __DJGPP__
#include <time.h>
#include <string.h>
#else
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#endif

int inpselect(int n, fd_set *set, struct timeval *tmout);
