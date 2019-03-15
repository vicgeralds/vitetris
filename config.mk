# config.mk
# Included in Makefile and src/Makefile.
# This file will be overwritten by the configure script.

prefix = c:\vitetris
bindir = $(prefix)
docdir = $(prefix)

#SHELL = sh
EXE = .exe

#CC = tcc
#MODEL = -ml
#CFLAGS = $(MODEL) -O

TWOPLAYER = y
#JOYSTICK = y
#NETWORK = y
MENU = y
BLOCKSTYLES = y

#INPUT_SYS = conio
CURSES = y
#ALLEGRO = y
LDFLAGS = $(MODEL)
LDLIBS  = -lpdcurses -lemu $(LIBS)

# DOS millisecond granularity
#PCTIMER = pctime14/gccint8
#PCTIMER_INC = -I../pctime14

