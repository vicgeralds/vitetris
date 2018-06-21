/* should be included from config.h */

#if defined(WIN32) || defined(__WIN32__)
#undef WIN32
#define WIN32 1
#define WIN32_LEAN_AND_MEAN

#elif defined(MSDOS) || defined(__MSDOS__) || defined(__PACIFIC__)
#undef MSDOS
#define MSDOS 1

#else
#undef UNIX
#define UNIX 1
#endif

#if UNIX
#define CONFIG_FILENAME ".vitetris"
#else
#define CONFIG_FILENAME "vitetris.cfg"
#endif
 
/* Only used if UNIX */
#ifndef HISCORE_FILENAME
#define HISCORE_FILENAME "/var/games/vitetris-hiscores"
#endif

#if __DJGPP__ || __TURBOC__
#undef HAVE_CONIO_H
#undef HAVE_GETTEXTINFO
#define HAVE_CONIO_H 1
#define HAVE_GETTEXTINFO 1
#endif
