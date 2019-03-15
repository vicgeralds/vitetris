#
# src\textgfx\tc.mk

OBJS = term.obj block.obj win.obj print.obj ibmgfx.obj \
       $(BACKEND).obj $(BACKEND)_win.obj
curses_win = curse$(lfn83_s_win)

all: textgfx.lib $(BACKEND)

textgfx.lib: $(OBJS)
	del textgfx.lib
	$(LIBEXE) textgfx.lib +term.obj +block.obj +win.obj +print.obj +ibmgfx.obj

ansi:
	$(LIBEXE) textgfx.lib +ansi.obj +ansi_win.obj

curses:
	$(LIBEXE) textgfx.lib +curses.obj +$(curses_win).obj

term.obj: term.c textgfx.h ..\options.h ..\config.h
	$(CC) $(CCFLAGS) -I.. -c term.c

block.obj: block.c textgfx.h ..\options.h Makefile.TC
	$(CC) $(CCFLAGS)      $(DNO_BLOCKSTYLES) -c block.c

win.obj: win.c textgfx.h ..\draw\draw.h ..\game\tetris.h Makefile.TC
	$(CC) $(CCFLAGS) -I.. $(DTWOPLAYER) $(DNO_MENU) -c win.c

print.obj: print.c textgfx.h
	$(CC) $(CCFLAGS)  -c print.c

ibmgfx.obj: ibmgfx.c
	$(CC) $(CCFLAGS)  -c ibmgfx.c

ansi.obj: ansi.c ansivt.h textgfx.h ..\input\termin.h Makefile.TC
	$(CC) $(CCFLAGS) -I.. $(DNO_MENU) $(DNO_BLOCKSTYLES) -c ansi.c

ansi_win.obj: ansi_win.c ansivt.h textgfx.h \
		..\game\tetris.h ..\draw\draw.h Makefile.TC
	$(CC) $(CCFLAGS) -I.. $(DTWOPLAYER) $(DNO_MENU) -c ansi_win.c

curses.obj: curses.c curs.h textgfx.h ..\input\termin.h Makefile.TC
	$(CC) $(CCFLAGS) $(CURSES_INC) $(DNO_BLOCKSTYLES) -c curses.c

curses_win.obj: $(curses_win).obj
$(curses_win).obj: $(curses_win).c curs.h \
		..\game\tetris.h ..\draw\draw.h Makefile.TC
	$(CC) $(CCFLAGS) -I.. $(CURSES_INC) $(DTWOPLAYER) $(DNO_MENU) -c $(curses_win).c

Makefile.TC: ..\src-conf.mk lfn83.mk tc.mk
	copy ..\src-conf.mk+lfn83.mk+tc.mk Makefile.TC

lfn83.mk: tcmake.bat
	tcmake lfn83

clean:
	del textgfx.lib
	del textgfx.bak
	del *.obj
	del Makefile.TC
	del lfn83.mk
