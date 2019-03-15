#
# src\input\tc.mk

OBJS = input.obj keyboard.obj

default: $(BACKEND)

conio: $(OBJS) inp_con.obj
	del input.lib
	$(LIBEXE) input.lib +input.obj +keyboard.obj +inp_con.obj

curses: $(OBJS) inp_curses.obj
	del input.lib
	$(LIBEXE) input.lib +input.obj +keyboard.obj +inp_c$(lfn83_urses).obj

input.obj: input.c input.h keyboard.h ..\timer.h ..\draw\draw.h \
	   ..\game\tetris.h ..\netw\sock.h ..\textgfx\textgfx.h Makefile.TC
	$(CC) $(CCFLAGS) -I.. $(DTWOPLAYER) $(DSOCKET) $(DNO_MENU) -c input.c

keyboard.obj: keyboard.c keyboard.h input.h escseq.h Makefile.TC
	$(CC) $(CCFLAGS) -I.. $(DTWOPLAYER) $(DCURSES) -c keyboard.c

inp_curses.obj: inp_c$(lfn83_urses).obj
inp_c$(lfn83_urses).obj: inp_c$(lfn83_urses).c termin.h keyboard.h input.h
	$(CC) $(CCFLAGS) $(CURSES_INC) -c inp_c$(lfn83_urses).c

inp_con.obj: inp_con.c termin.h keyboard.h input.h
	$(CC) $(CCFLAGS) -c inp_con.c

Makefile.TC: ..\src-conf.mk lfn83.mk tc.mk
	copy ..\src-conf.mk+lfn83.mk+tc.mk Makefile.TC

lfn83.mk: tcmake.bat
	tcmake lfn83

clean:
	del input.lib
	del *.obj
	del Makefile.TC
	del lfn83.mk
