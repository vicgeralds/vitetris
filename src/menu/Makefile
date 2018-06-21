include ../src-conf.mk

OBJS = menucore.o dropdown.o inputset.o inputdev.o \
       gamemenu.o gm_2p.o gm_sock.o \
       hslist.o gameover.o

EXT_OBJS = startup.o invit.o netplay.o \
	   optsmenu.o blockstyle.o men_$(BACKEND).o

HEADERS = menu.h ../input/input.h ../textgfx/textgfx.h

all: menu.a $(menuext_lib)

menu.a: $(OBJS)
	ar rs menu.a $(OBJS)

menuext.a: $(EXT_OBJS)
	rm -f menuext.a
	ar rcs menuext.a $(EXT_OBJS)

menucore.o: menucore.c $(HEADERS) ../src-conf.mk
	$(CC) $(CCFLAGS) $(DNO_MENU) -c menucore.c 

dropdown.o: dropdown.c $(HEADERS) ../draw/draw.h
	$(CC) $(CCFLAGS)  -c dropdown.c

inputset.o: inputset.c $(HEADERS) internal.h \
	    ../draw/draw.h ../lang.h ../src-conf.mk
	$(CC) $(CCFLAGS) $(DTWOPLAYER) $(DJOYSTICK) $(DNO_MENU) -c inputset.c

inputdev.o: inputdev.c $(HEADERS) internal.h ../options.h ../src-conf.mk
	$(CC) $(CCFLAGS) $(DTWOPLAYER) $(DNO_MENU)  -c inputdev.c

gamemenu.o: gamemenu.c menu.h internal.h ../input/input.h ../game/tetris.h \
	    ../lang.h ../options.h ../src-conf.mk
	$(CC) $(CCFLAGS) -I.. $(DTWOPLAYER) $(DJOYSTICK) $(DSOCKET) -c gamemenu.c

gm_2p.o: gm_2p.c $(HEADERS) internal.h ../draw/draw.h
	$(CC) $(CCFLAGS)      -c gm_2p.c

gm_sock.o: gm_sock.c $(HEADERS) internal.h ../netw/sock.h ../draw/draw.h \
					 ../game/tetris.h ../game/game.h
	$(CC) $(CCFLAGS) -I.. -c gm_sock.c

hslist.o: hslist.c $(HEADERS) ../hiscore.h
	$(CC) $(CCFLAGS) -I.. -c hslist.c

gameover.o: gameover.c $(HEADERS) ../hiscore.h ../draw/draw.h ../game/tetris.h
	$(CC) $(CCFLAGS) -I.. -c gameover.c

startup.o: startup.c menu.h menuext.h ../options.h ../game/tetris.h \
							../src-conf.mk
	$(CC) $(CCFLAGS) -I.. -c startup.c $(DINET)

invit.o: invit.c $(HEADERS) menuext.h ../draw/draw.h ../netw/sock.h
	$(CC) $(CCFLAGS)  -c invit.c

netplay.o: netplay.c $(HEADERS) menuext.h ../netw/sock.h ../options.h
	$(CC) $(CCFLAGS)  -c netplay.c

optsmenu.o: optsmenu.c menu.h menuext.h ../input/input.h ../textgfx/textgfx.h \
	    ../draw/draw.h ../options.h ../lang.h ../src-conf.mk
	$(CC) $(CCFLAGS) $(DTWOPLAYER) $(DNO_BLOCKSTYLES) -c optsmenu.c

blockstyle.o: blockstyle.c menu.h menuext.h ../textgfx/textgfx.h ../options.h
	$(CC) $(CCFLAGS)  -c blockstyle.c

men_ansi.o: men_ansi.c menu.h menuext.h ../draw/draw.h ../game/tetris.h \
	    ../textgfx/textgfx.h ../textgfx/ansivt.h ../lang.h ../src-conf.mk
	$(CC) $(CCFLAGS) -I.. $(DTWOPLAYER) $(DTTY_SOCKET) $(DINET) $(DNO_BLOCKSTYLES) -c men_ansi.c

men_curses.o: men_curses.c menu.h menuext.h ../draw/draw.h ../game/tetris.h \
	      ../textgfx/textgfx.h ../textgfx/curs.h ../src-conf.mk
	$(CC) $(CCFLAGS) -I.. $(CURSES_INC) $(DTWOPLAYER) $(DINET) $(DTTY_SOCKET) $(DNO_BLOCKSTYLES) -c men_curses.c

men_allegro.o: men_allegro.c menu.h menuext.h ../draw/draw.h ../game/tetris.h \
	       ../textgfx/textgfx.h ../input/input.h ../options.h \
	       ../config.h ../src-conf.mk
	$(CC) $(CCFLAGS) -I.. $(DTWOPLAYER) $(DTTY_SOCKET) $(DINET) $(DNO_BLOCKSTYLES) -c men_allegro.c

clean:
	rm -f menu.a menuext.a $(OBJS)
	rm -f $(EXT_OBJS)
	rm -f men_ansi.o men_curses.o men_allegro.o
	rm -f block~wv.o men_c~9i.o men_a~ol.o

.PHONY: all clean
