#
# src\menu\tc.mk

OBJS = menucore.obj dropdown.obj inputset.obj inputdev.obj \
       gamemenu.obj gm_2p.obj hslist.obj gameover.obj
#      gm_sock.obj

blockstyle = block$(lfn83_style)
men_curses = men_c$(lfn83_urses)

EXT_OBJS = startup.obj optsmenu.obj $(blockstyle).obj men_$(BACKEND).obj

HEADERS = menu.h ..\input\input.h ..\textgfx\textgfx.h

all: menu.lib $(menuext_lib) $(BACKEND)

menu.lib: $(OBJS)
	del menu.lib
	$(LIBEXE) menu.lib +menucore.obj +dropdown.obj +inputset.obj +inputdev.obj +gamemenu.obj +gm_2p.obj +hslist.obj +gameover.obj
#	$(LIBEXE) menu.lib +gm_sock.obj

menuext.lib: $(EXT_OBJS)
	del menuext.lib
	$(LIBEXE) menuext.lib +startup.obj +optsmenu.obj +$(blockstyle).obj

ansi:
	$(LIBEXE) menuext.lib +men_ansi.obj
curses:
	$(LIBEXE) menuext.lib +$(men_curses).obj

menucore.obj: menucore.c $(HEADERS) Makefile.TC
	$(CC) $(CCFLAGS) $(DNO_MENU) -c menucore.c 

dropdown.obj: dropdown.c $(HEADERS) ..\draw\draw.h
	$(CC) $(CCFLAGS) -c dropdown.c

inputset.obj: inputset.c $(HEADERS) internal.h \
	      ..\draw\draw.h ..\lang.h Makefile.TC
	$(CC) $(CCFLAGS) $(DTWOPLAYER) $(DNO_MENU) -c inputset.c

inputdev.obj: inputdev.c $(HEADERS) internal.h ..\options.h Makefile.TC
	$(CC) $(CCFLAGS) $(DTWOPLAYER) $(DNO_MENU) -c inputdev.c

gamemenu.obj: gamemenu.c menu.h internal.h ..\input\input.h ..\game\tetris.h \
	      ../lang.h ../options.h Makefile.TC
	$(CC) $(CCFLAGS) -I.. $(DTWOPLAYER) -c gamemenu.c

gm_2p.obj: gm_2p.c $(HEADERS) internal.h ..\draw\draw.h
	$(CC) $(CCFLAGS) -c gm_2p.c

gm_sock.obj: gm_sock.c $(HEADERS) internal.h ..\netw\sock.h \
					     ..\draw\draw.h ..\game\tetris.h
	$(CC) $(CCFLAGS) -I.. -c gm_sock.c

hslist.obj: hslist.c $(HEADERS) ..\hiscore.h
	$(CC) $(CCFLAGS) -I.. -c hslist.c

gameover.obj: gameover.c $(HEADERS) ..\hiscore.h ..\draw\draw.h ..\game\tetris.h
	$(CC) $(CCFLAGS) -I.. -c gameover.c

startup.obj: startup.c menu.h menuext.h ..\options.h ..\game\tetris.h
	$(CC) $(CCFLAGS) -I.. -c startup.c

optsmenu.obj: optsmenu.c menu.h menuext.h ..\input\input.h \
  ..\textgfx\textgfx.h ..\draw/draw.h ..\options.h ..\lang.h Makefile.TC
	$(CC) $(CCFLAGS) $(DTWOPLAYER) $(DNO_BLOCKSTYLES) -c optsmenu.c

$(blockstyle).obj: $(blockstyle).c menu.h menuext.h ..\textgfx\textgfx.h \
						    ..\options.h
	$(CC) $(CCFLAGS) -c $(blockstyle).c

men_ansi.obj: men_ansi.c menu.h menuext.h ..\draw\draw.h ..\game\tetris.h \
	    ..\textgfx\textgfx.h ..\textgfx\ansivt.h ..\lang.h Makefile.TC
	$(CC) $(CCFLAGS) -I.. $(DTWOPLAYER) $(DNO_BLOCKSTYLES) -c men_ansi.c

men_curses.obj: $(men_curses).obj
$(men_curses).obj: $(men_curses).c menu.h menuext.h ..\draw\draw.h \
  ..\game\tetris.h ..\textgfx\textgfx.h ..\textgfx\curs.h Makefile.TC
	$(CC) $(CCFLAGS) -I.. $(CURSES_INC) $(DTWOPLAYER) $(DNO_BLOCKSTYLES) -c $(men_curses).c

Makefile.TC: ..\src-conf.mk lfn83.mk tc.mk
	copy ..\src-conf.mk+lfn83.mk+tc.mk Makefile.TC

lfn83.mk: tcmake.bat
	tcmake lfn83

clean:
	del menu.lib
	del menuext.lib
	del menuext.bak
	del *.obj
	del Makefile.TC
	del lfn83.mk
