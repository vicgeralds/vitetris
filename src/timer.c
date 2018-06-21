#include "config.h"
#if UNIX
#include <sys/time.h>
#include <unistd.h>
#elif WIN32
#include <windows.h>
#include <mmsystem.h>
#elif PCTIMER
#include "pctimer.h"
#elif ALLEGRO
#include <allegro/timer.h>
#else
#include <time.h>
#include <dos.h>
#endif
#include "timer.h"

#ifdef ALLEGRO_VERSION
static volatile unsigned long elapsed_tm;
static void inc_tm() { elapsed_tm++; }
END_OF_FUNCTION(inc_tm)
#endif

void timer_init()
{
#if WIN32
	timeBeginPeriod(1);	/* sleep just 1 millisecond */
#elif PCTIMER
	pctimer_init(1000);
#endif
}

void timer_end()
{
#if WIN32
	timeEndPeriod(1);
#elif PCTIMER
	pctimer_exit();
#endif
}

int gettm(int a)
{
	int b;
#if UNIX
	struct timeval tmv;
	gettimeofday(&tmv, NULL);
	b = 1000*(tmv.tv_sec % 10) + tmv.tv_usec/1000;
#elif WIN32
	long ms = timeGetTime();
	b = ms % 10000;
#elif PCTIMER
	unsigned long ms = pctimer_time(0, pctimer_get_ticks());
	b = ms % 10000;
#elif ALLEGRO
	if (!elapsed_tm) {
		elapsed_tm = 1;
		install_int(inc_tm, 1);
	}
	b = elapsed_tm % 10000;
#elif UCLOCKS_PER_SEC
	uclock_t ms = uclock() / (UCLOCKS_PER_SEC/1000);
	b = ms % 10000;
#else
	clock_t hs = clock() / (CLOCKS_PER_SEC/100.0);
	b = 10*(hs % 1000);
#endif
	b++;
	while (b < a)
		b += 10000;
	return b;
}

void sleep_msec(unsigned ms)
{
#if UNIX
	usleep(1000*ms);
#elif WIN32
	Sleep(ms);
#elif PCTIMER
	pctimer_sleep(ms);
#elif ALLEGRO
	if (!elapsed_tm) {
		elapsed_tm = 1;
		install_int(inc_tm, 1);
	}
	rest(ms);
#else
	delay(ms);
#endif
}
