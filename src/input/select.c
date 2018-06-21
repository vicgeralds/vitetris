#include "select.h"
#include <errno.h>
#include <stdio.h>	/* perror */
#include <stdlib.h>	/* exit */
#include "input.h"
#include "../textgfx/textgfx.h"

int inpselect(int n, fd_set *set, struct timeval *tmout)
{
	n = select(n, set, NULL, NULL, tmout);
	if (n == -1 && errno != EINTR) {
		textgfx_end();
		perror("Error waiting for input");
		exit(1);
	}
	return n;
}

int waitinput(int fd, unsigned ms)
{
	fd_set set;
	struct timeval tmv;
	int ret;
	if (fd < 0)
		return 0;
	do {
		FD_ZERO(&set);
		FD_SET(fd, &set);
		tmv.tv_sec = 0;
		tmv.tv_usec = 1000*ms;
		ret = inpselect(fd+1, &set, &tmv);
	} while (ret == -1);
	return ret;
}
