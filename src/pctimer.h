#if __TURBOC__
#include "tcint8.h"
#else
#include "gccint8.h"
#endif

#if __TURBOC__ || __DJGPP__ < 2
#define pctimer_init(Hz) \
	init8h(Hz); ticks_8h = 0
#define pctimer_exit() \
	quit8h()
#define pctimer_get_ticks() \
	ticks_8h
#define pctimer_time(start,stop) \
	time8h(start, stop)
#define pctimer_sleep(ms) \
	delay8h(ms)
#endif
