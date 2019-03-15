#
# src\tc.mk
# 
# Makefile for Turbo C (run tcmake.bat).
#
# All source files MUST be converted to DOS line endings.
# If not, tcc will perhaps not warn you and build a useless obj file.
# (Then you get lots of undefined symbols.)

OBJS = main.obj cmdline.obj cfgfile.obj options.obj hiscore.obj lang.obj \
       timer.obj $(pctimer_obj)

tetris.exe: main.lib libs Makefile.TC
	$(CC) -etetris.exe $(LDFLAGS) main.lib $(pctimer_obj) game.lib menu.lib $(menuext_lib) input.lib draw.lib textgfx.lib $(LDLIBS)

main.lib: $(OBJS)
	del main.lib
	$(LIBEXE) main.lib +main.obj +cmdline.obj +cfgfile.obj +options.obj +hiscore.obj +lang.obj +timer.obj

main.obj: main.c timer.h cfgfile.h options.h lang.h \
	  textgfx\textgfx.h input\input.h game\tetris.h game\game.h \
	  menu\menuext.h netw\sock.h Makefile.TC
	$(CC) $(CCFLAGS) -I. $(DTWOPLAYER) $(DSOCKET) $(DNO_MENU) -c main.c

#cmdline.obj: cmdli$(lfn83_ne_empty).c Makefile.TC
#	$(CC) $(CCFLAGS) -c -ocmdline.obj cmdli$(lfn83_ne_empty).c

cmdline.obj: cmdline.c version.h config.h config2.h options.h cfgfile.h \
	     lang.h game\game.h textgfx\textgfx.h Makefile.TC
	$(CC) $(CCFLAGS) $(DTWOPLAYER) $(DNO_MENU) $(DNO_BLOCKSTYLES) -c cmdline.c

cfgfile.obj: cfgfile.c cfgfile.h options.h hiscore.h \
	     input\input.h input\keyboard.h draw\draw.h Makefile.TC
	$(CC) $(CCFLAGS) -I. $(DTWOPLAYER) $(DCURSES) -c cfgfile.c 

options.obj: options.c options.h
	$(CC) $(CCFLAGS) -c options.c

hiscore.obj: hiscore.c hiscore.h cfgfile.h lang.h game\tetris.h
	$(CC) $(CCFLAGS) -I. -c hiscore.c 

lang.obj: lang.c lang.h
	$(CC) $(CCFLAGS) -c lang.c

timer.obj: timer.c timer.h pctimer.h config.h config2.h Makefile.TC
	$(CC) $(CCFLAGS) $(DPCTIMER) $(PCTIMER_INC) -c timer.c

pctimer.obj: ..\$(PCTIMER).c ..\$(PCTIMER).h Makefile.TC
	$(CC) $(CCFLAGS) $(PCTIMER_INC) -c -opctimer.obj ..\$(PCTIMER).c

libs:
	tcmake libs $(INPUT_SYS)

Makefile.TC: ..\config.mk src-conf.mk lfn83.mk tc.mk
	copy ..\config.mk+src-conf.mk+lfn83.mk+tc.mk Makefile.TC

lfn83.mk: tcmake.bat
	tcmake lfn83

clean:
	del tetris.exe
	del *.obj
	del *.lib
	del *.bak
	del Makefile.TC
	del lfn83.mk
	cd game
	$(MAKE) -ftc.mk clean
	cd ..\menu
	$(MAKE) -ftc.mk clean
	cd ..\input
	$(MAKE) -ftc.mk clean
	cd ..\draw
	$(MAKE) -ftc.mk clean
	cd ..\textgfx
	$(MAKE) -ftc.mk clean
	cd ..
