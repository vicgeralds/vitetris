# src\src-conf.mk
CC = gcc
CFLAGS = -O2
#CFLAGS = -O2 -Wall -pedantic -Wno-parentheses
CCFLAGS = $(CFLAGS) $(CPPFLAGS)
OBJEXT = .o
LIBEXT = .a
DTWOPLAYER = -DTWOPLAYER=1
tetris2p_obj = tetris2p$(OBJEXT)
#DJOYSTICK = -DJOYSTICK=1
BACKEND = curses
DCURSES = -DCURSES=1
#BACKEND = ansi
#BACKEND = allegro
#DALLEGRO = -DALLEGRO=1
#DNO_MENU = -DNO_MENU=1
menuext_lib = menuext$(LIBEXT)
#DNO_BLOCKSTYLES = -DNO_BLOCKSTYLES=1
#DPCTIMER = -DPCTIMER=1
#pctimer_obj = pctimer$(OBJEXT)
