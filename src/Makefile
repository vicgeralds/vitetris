include ../config.mk
include src-conf.mk

OBJS = main.o cmdline.o cfgfile.o options.o hiscore.o lang.o \
       timer.o $(pctimer_obj) focus.o
#OBJS += ../icon.o

tetris: $(OBJS) libs ../config.mk
	$(CC) -o tetris $(OBJS) game.a $(menuext_lib) menu.a $(netw_lib) input.a draw.a textgfx.a $(LDFLAGS) $(LDLIBS)

main.o: main.c timer.h cfgfile.h options.h lang.h focus.h \
	textgfx/textgfx.h input/input.h game/tetris.h game/game.h \
	menu/menuext.h netw/sock.h config.h src-conf.mk
	$(CC) $(CCFLAGS) -I. $(DTWOPLAYER) $(DSOCKET) $(DINET) $(DTTY_SOCKET) $(DNO_MENU) $(DTERM_RESIZING) $(DXLIB) $(DALLEGRO) -c main.c

#cmdline.o: cmdline-empty.c
#	$(CC) $(CCFLAGS) -c -ocmdline.o cmdline-empty.c 

cmdline.o: cmdline.c version.h config.h config2.h options.h cfgfile.h \
	   lang.h game/game.h textgfx/textgfx.h src-conf.mk
	$(CC) $(CCFLAGS) -I. $(DTWOPLAYER) $(DJOYSTICK) $(DCURSES) $(DALLEGRO) $(DSOCKET) $(DINET) $(DTTY_SOCKET) $(DNO_MENU) $(DNO_BLOCKSTYLES) $(DHISCORE_FILENAME) -c cmdline.c

cfgfile.o: cfgfile.c cfgfile.h options.h hiscore.h input/input.h \
	   input/keyboard.h input/joystick.h draw/draw.h src-conf.mk
	$(CC) $(CCFLAGS) -I. $(DTWOPLAYER) $(DJOYSTICK) $(DCURSES) $(DALLEGRO) -c cfgfile.c 

options.o: options.c options.h

hiscore.o: hiscore.c hiscore.h cfgfile.h lang.h game/tetris.h src-conf.mk
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

libs: gamea menua netwa inputa drawa textgfxa
	mv -f game/game.a .
	mv -f menu/*.a .
	-mv -f netw/netw.a .
	mv -f input/input.a .
	mv -f draw/draw.a .
	mv -f textgfx/textgfx.a .

gamea:
	$(MAKE) -Cgame
menua:
	$(MAKE) -Cmenu
netwa:
	$(MAKE) -Cnetw
inputa:
	$(MAKE) -Cinput $(INPUT_SYS)
drawa:
	$(MAKE) -Cdraw
textgfxa:
	$(MAKE) -Ctextgfx

clean:
	rm -f tetris tetris.exe $(OBJS) pctimer.o
	rm -f game.a menu.a menuext.a netw.a input.a draw.a textgfx.a
	$(MAKE) -Cgame clean
	$(MAKE) -Cmenu clean
	$(MAKE) -Cnetw clean
	$(MAKE) -Cinput clean
	$(MAKE) -Cdraw clean
	$(MAKE) -Ctextgfx clean

.PHONY: libs gamea menua netwa inputa drawa textgfxa clean
