include ../src-conf.mk

OBJS = menucore.o dropdown.o \
       men_$(BACKEND).o menus.o invit.o netplay.o gm_sock.o \
       hslist.o gameover.o inputset.o inputdev.o \
       optsmenu.o blockstyle.o

HEADERS = menu.h ../input/input.h ../textgfx/textgfx.h

menu.a: $(OBJS)
	rm -f menu.a
	ar rcs menu.a $(OBJS)

menucore.o: menucore.c $(HEADERS)
	$(CC) $(CCFLAGS)  -c menucore.c 

dropdown.o: dropdown.c $(HEADERS) ../draw.h
	$(CC) $(CCFLAGS)  -c dropdown.c

menus.o: menus.c $(HEADERS) ../draw.h ../options.h ../game/tetris.h \
			    ../lang.h ../src-conf.mk
	$(CC) $(CCFLAGS) -I.. $(DJOYSTICK) $(DSOCKET) $(DINET) -c menus.c 

invit.o: invit.c $(HEADERS) ../draw.h ../net/sock.h
	$(CC) $(CCFLAGS)  -c invit.c

netplay.o: netplay.c $(HEADERS) ../net/sock.h ../options.h
	$(CC) $(CCFLAGS)  -c netplay.c

gm_sock.o: gm_sock.c $(HEADERS) ../draw.h ../game/tetris.h ../game/game.h \
				../net/sock.h
	$(CC) $(CCFLAGS) -I.. -c gm_sock.c

hslist.o: hslist.c $(HEADERS) ../hiscore.h
	$(CC) $(CCFLAGS) -I.. -c hslist.c

gameover.o: gameover.c $(HEADERS) ../hiscore.h ../draw.h ../game/tetris.h
	$(CC) $(CCFLAGS) -I.. -c gameover.c

inputset.o: inputset.c $(HEADERS) ../draw.h ../lang.h ../src-conf.mk
	$(CC) $(CCFLAGS) $(DJOYSTICK) -c inputset.c

inputdev.o: inputdev.c $(HEADERS) ../options.h
	$(CC) $(CCFLAGS)  -c inputdev.c

optsmenu.o: optsmenu.c $(HEADERS) ../draw.h ../options.h ../lang.h
	$(CC) $(CCFLAGS)  -c optsmenu.c

blockstyle.o: blockstyle.c menu.h ../textgfx/textgfx.h ../options.h
	$(CC) $(CCFLAGS)  -c blockstyle.c

men_ansi.o: men_ansi.c menu.h ../draw.h ../game/tetris.h \
	    ../textgfx/textgfx.h ../lang.h ../config2.h ../src-conf.mk
	$(CC) $(CCFLAGS) -I.. $(DTTY_SOCKET) $(DINET) -c men_ansi.c

men_curses.o: men_curses.c menu.h ../draw.h ../game/tetris.h \
	      ../textgfx/textgfx.h ../src-conf.mk
	$(CC) $(CCFLAGS) -I.. $(CURSES_INC) $(DINET) $(DTTY_SOCKET) -c men_curses.c

men_allegro.o: men_allegro.c menu.h ../draw.h ../game/tetris.h \
	       ../textgfx/textgfx.h ../input/input.h ../options.h \
	       ../config.h ../src-conf.mk
	$(CC) $(CCFLAGS) -I.. $(DTTY_SOCKET) $(DINET) -c men_allegro.c

clean:
	rm -f menu.a $(OBJS)
	rm -f men_ansi.o men_curses.o men_allegro.o
	rm -f block~wv.o men_c~9i.o men_a~ol.o

.PHONY: all clean
