#include <stdio.h>
#include <signal.h>

#if __MINGW32__
#define ENVIR "mingw "
#elif __DJGPP__
#define ENVIR "djgpp "
#elif __TURBOC__
#define ENVIR "turboc "
#elif __PACIFIC__
#define MSDOS
#define ENVIR "pacific "
#else
#define ENVIR ""
#endif

int main()
{
#if defined(WIN32) || defined(__WIN32__)
	puts("win32 "ENVIR"_");
#elif defined(MSDOS) || defined(__MSDOS__)
	puts("msdos "ENVIR"_");
#elif __CYGWIN__
	puts("cygwin _");
#else
	puts("unix _");
#endif
#ifdef SIGWINCH
	puts("SIGWINCH");
#endif
	return 0;
}
