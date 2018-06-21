include ../src-conf.mk

OBJS = block.o win.o print.o ibmgfx.o \
       $(BACKEND).o $(BACKEND)_win.o term.o resize.o

textgfx.a: $(OBJS)
	rm -f textgfx.a
	ar rcs textgfx.a $(OBJS)

term.o: term.c textgfx.h ../options.h ../config.h
	$(CC) $(CCFLAGS) -I.. -c term.c

block.o: block.c textgfx.h ../options.h ../src-conf.mk
	$(CC) $(CCFLAGS) $(DNO_BLOCKSTYLES) -c block.c

win.o: win.c textgfx.h ../draw/draw.h ../game/tetris.h ../src-conf.mk
	$(CC) $(CCFLAGS) -I.. $(DTWOPLAYER) $(DNO_MENU) -c win.c

print.o: print.c textgfx.h
	$(CC) $(CCFLAGS)  -c print.c

ibmgfx.o: ibmgfx.c
	$(CC) $(CCFLAGS)  -c ibmgfx.c

ansi.o: ansi.c ansivt.h textgfx.h ../input/termin.h ../src-conf.mk
	$(CC) $(CCFLAGS) -I.. $(DNO_MENU) $(DNO_BLOCKSTYLES) -c ansi.c

ansi_win.o: ansi_win.c ansivt.h textgfx.h \
		../game/tetris.h ../draw/draw.h ../src-conf.mk
	$(CC) $(CCFLAGS) -I.. $(DTWOPLAYER) $(DNO_MENU) -c ansi_win.c

curses.o: curses.c curs.h textgfx.h ../input/termin.h
	$(CC) $(CCFLAGS) $(CURSES_INC) -c curses.c

curses_win.o: curses_win.c curs.h textgfx.h \
		../game/tetris.h ../draw/draw.h ../src-conf.mk
	$(CC) $(CCFLAGS) -I.. $(CURSES_INC) $(DTWOPLAYER) $(DNO_MENU) -c curses_win.c

allegro.o: allegro.c textgfx.h alleg.h ../options.h ../lang.h ../config.h
	$(CC) $(CCFLAGS) -I.. -c allegro.c

allegro_win.o: allegro_win.c textgfx.h alleg.h allegro_icon.h \
		../game/tetris.h ../draw/draw.h ../config.h
	$(CC) $(CCFLAGS) -I.. -c allegro_win.c

resize.o: resize.c textgfx.h curs.h \
		../game/tetris.h ../draw/draw.h ../src-conf.mk
	$(CC) $(CCFLAGS) -I.. $(CURSES_INC) $(DTWOPLAYER) $(DNO_MENU) $(DCURSES) $(DTERM_RESIZING) $(DSOCKET) -c resize.c

clean:
	rm -f textgfx.a $(OBJS)
	rm -f ansi.o ansi_win.o curses.o curses_win.o allegro.o allegro_win.o
	rm -f curse~60.o alleg~jn.o

.PHONY: clean
