# config.mk
# Included in Makefile and src/Makefile.
# This file will be overwritten by the configure script.

prefix = /usr/local
bindir = $(prefix)/bin
datarootdir = $(prefix)/share
docdir     = $(datarootdir)/doc/vitetris
pixmapdir  = $(datarootdir)/pixmaps
desktopdir = $(datarootdir)/applications

datadir = $(datarootdir)/allegro
#datadir = $(datarootdir)/allegro/vitetris

UNIX = y
#EXE = .exe

TWOPLAYER = y
#JOYSTICK = y
NETWORK = y
TTY_SOCKET = y
TERM_RESIZING = y
MENU = y
BLOCKSTYLES = y

INPUT_SYS = unixterm
#CURSES = y
#ALLEGRO = y
#XLIB = y
#XLIB_INC =
LDFLAGS =
LDLIBS  = $(LIBS)

# DOS millisecond granularity
#PCTIMER = pctime14/gccint8
#PCTIMER_INC = -I../pctime14

