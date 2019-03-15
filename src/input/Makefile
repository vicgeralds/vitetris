include ../src-conf.mk

OBJS = input.o keyboard.o joystick.o playerin.o

default: $(BACKEND)

con: $(OBJS) inp_con.o
	rm -f input.a
	ar rcs input.a $(OBJS) inp_con.o

curses: $(OBJS) inp_curses.o $(joylinux_obj) $(select_obj)
	rm -f input.a
	ar rcs input.a $(OBJS) inp_curses.o $(joylinux_obj) $(select_obj)

allegro: $(OBJS) inp_allegro.o $(select_obj)
	rm -f input.a
	ar rcs input.a $(OBJS) inp_allegro.o $(select_obj)

input.o: input.c input.h keyboard.h joystick.h \
	 ../timer.h ../draw/draw.h ../game/tetris.h ../netw/sock.h \
	 ../textgfx/textgfx.h ../focus.h ../src-conf.mk
	$(CC) $(CCFLAGS) -I.. $(DTWOPLAYER) $(DJOYSTICK) $(DSOCKET) $(DTTY_SOCKET) $(DNO_MENU) $(DTERM_RESIZING) $(DXLIB) $(DALLEGRO) -c input.c

keyboard.o: keyboard.c keyboard.h input.h escseq.h \
		../config.h ../config2.h ../src-conf.mk
	$(CC) $(CCFLAGS) -I.. $(DTWOPLAYER) $(DCURSES) $(DALLEGRO) -c keyboard.c

joystick.o: joystick.c joystick.h input.h ../timer.h

inp_curses.o: inp_curses.c termin.h  keyboard.h  input.h
	$(CC) $(CCFLAGS) $(CURSES_INC) -c inp_curses.c

inp_con.o   : inp_con.c    termin.h  keyboard.h  input.h

inp_allegro.o: inp_allegro.c keyboard.h joystick.h input.h \
		../timer.h ../game/tetris.h ../textgfx/textgfx.h \
		../textgfx/alleg.h ../config.h
	$(CC) $(CCFLAGS) -I.. -c inp_allegro.c

playerin.o: playerin.c input.h ../options.h

.c.o:
	$(CC) $(CCFLAGS) -c $<

clean:
	rm -f input.a *.o
	rm -f inp_c~70.o inp_a~m7.o

.PHONY: default curses con allegro clean
