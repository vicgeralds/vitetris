include ../src-conf.mk

OBJS = block.o win.o print.o ibmgfx.o $(BACKEND).o term.o resize.o

textgfx.a: $(OBJS)
	rm -f textgfx.a
	ar rcs textgfx.a $(OBJS)

term.o: term.c textgfx.h ../options.h ../config.h
	$(CC) $(CCFLAGS) -I.. -c term.c

block.o: block.c textgfx.h ../options.h
	$(CC) $(CCFLAGS) -c block.c

win.o: win.c textgfx.h ../draw.h
	$(CC) $(CCFLAGS) -I.. -c win.c

print.o: print.c textgfx.h
	$(CC) $(CCFLAGS) -c print.c

ibmgfx.o: ibmgfx.c
	$(CC) $(CCFLAGS) -c ibmgfx.c

ansi.o: ansi.c textgfx.h ../game/tetris.h ../draw.h ../config2.h
	$(CC) $(CCFLAGS) -I.. -c ansi.c

curses.o: curses.c textgfx.h ../game/tetris.h ../draw.h
	$(CC) $(CCFLAGS) -I.. $(CURSES_INC) -c curses.c

allegro.o: allegro.c textgfx.h allegro_icon.h ../options.h ../lang.h \
    ../version.h ../input/keyboard.h ../game/tetris.h ../draw.h ../config.h
	$(CC) $(CCFLAGS) -I.. -c allegro.c

resize.o: resize.c textgfx.h ../game/tetris.h ../draw.h ../src-conf.mk
	$(CC) $(CCFLAGS) -I.. $(CURSES_INC) $(DCURSES) $(DTERM_RESIZING) $(DSOCKET) -c resize.c

clean:
	rm -f textgfx.a $(OBJS)
	rm -f ansi.o curses.o allegro.o

.PHONY: clean
