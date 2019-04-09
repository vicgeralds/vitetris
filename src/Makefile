include ../config.mk
include src-conf.mk

OBJS = main.o cmdline.o cfgfile.o options.o draw.o hiscore.o lang.o \
       timer.o $(pctimer_obj) focus.o
#OBJS += ../icon.o

tetris: $(OBJS) libs ../config.mk
	$(CC) -o tetris $(OBJS) game.a menu.a $(net_lib) input.a textgfx.a $(LDFLAGS) $(LDLIBS)

main.o: main.c timer.h cfgfile.h options.h lang.h focus.h \
	textgfx/textgfx.h input/input.h game/tetris.h game/game.h \
	menu/menu.h net/sock.h config.h src-conf.mk
	$(CC) $(CCFLAGS) -I. $(DSOCKET) $(DINET) $(DTTY_SOCKET) $(DTERM_RESIZING) $(DXLIB) $(DALLEGRO) -c main.c

#cmdline.o: cmdline-empty.c
#	$(CC) $(CCFLAGS) -c -ocmdline.o cmdline-empty.c 

cmdline.o: cmdline.c version.h config.h config2.h options.h cfgfile.h \
	   lang.h game/game.h textgfx/textgfx.h src-conf.mk
	$(CC) $(CCFLAGS) -I. $(DJOYSTICK) $(DCURSES) $(DALLEGRO) $(DSOCKET) $(DINET) $(DTTY_SOCKET) $(DHISCORE_FILENAME) -c cmdline.c

cfgfile.o: cfgfile.c cfgfile.h options.h hiscore.h input/input.h \
	   input/keyboard.h input/joystick.h draw.h src-conf.mk
	$(CC) $(CCFLAGS) -I. $(DJOYSTICK) $(DCURSES) $(DALLEGRO) -c cfgfile.c 

options.o: options.c options.h

draw.o: draw.c draw.h textgfx/textgfx.h game/tetris.h version.h hiscore.h \
	net/sock.h src-conf.mk
	$(CC) $(CCFLAGS) -I. $(DSOCKET) -c draw.c

hiscore.o: hiscore.c hiscore.h cfgfile.h options.h lang.h game/tetris.h \
							  src-conf.mk
	$(CC) $(CCFLAGS) -I. $(DHISCORE_FILENAME) -c hiscore.c 

lang.o: lang.c lang.h

timer.o: timer.c timer.h pctimer.h config.h config2.h ../config.mk src-conf.mk
	$(CC) $(CCFLAGS) $(DPCTIMER) $(PCTIMER_INC) $(DALLEGRO) -c timer.c

pctimer.o: ../$(PCTIMER).c ../$(PCTIMER).h ../config.mk src-conf.mk
	$(CC) $(CCFLAGS) $(PCTIMER_INC) -c -opctimer.o ../$(PCTIMER).c

focus.o: focus.c focus.h ../config.mk src-conf.mk
	$(CC) $(CCFLAGS) $(DXLIB) $(XLIB_INC) $(DALLEGRO) -c focus.c

../icon.o: ../icon.rc
	cd ..; windres icon.rc icon.o

.c.o:
	$(CC) $(CCFLAGS) -c $<

libs: gamea menua neta inputa textgfxa
	mv -f game/game.a .
	mv -f menu/*.a .
	-mv -f net/net.a .
	mv -f input/input.a .
	mv -f textgfx/textgfx.a .

gamea:
	cd game; $(MAKE)
menua:
	cd menu; $(MAKE)
neta:
	cd net; $(MAKE)
inputa:
	cd input; $(MAKE) $(INPUT_SYS)
textgfxa:
	cd textgfx; $(MAKE)

clean:
	rm -f tetris tetris.exe $(OBJS) pctimer.o
	rm -f game.a menu.a net.a input.a textgfx.a
	cd game; $(MAKE) clean
	cd menu; $(MAKE) clean
	cd net; $(MAKE) clean
	cd input; $(MAKE) clean
	cd textgfx; $(MAKE) clean

.PHONY: libs gamea menua neta inputa textgfxa clean
