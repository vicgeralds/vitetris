#
# src\game\tc.mk

OBJS = tetris.obj $(tetris2p_obj) game.obj
HEADERS = tetris.h ..\timer.h ..\input\input.h ..\draw\draw.h

game.lib: $(OBJS)
	del game.lib
	$(LIBEXE) game.lib +tetris.obj +tetris2p.obj +game.obj

tetris.obj: tetris.c $(HEADERS)
	$(CC) $(CCFLAGS) -I.. -c tetris.c

tetris2p.obj: tetris2p.c tetris2p.h $(HEADERS) ..\netw\sock.h Makefile.TC
	$(CC) $(CCFLAGS) -I.. $(DSOCKET) -c tetris2p.c

game.obj: game.c game.h tetris.h tetris2p.h ..\options.h ..\input\input.h \
	  ..\netw\sock.h ..\hiscore.h ..\menu\menu.h Makefile.TC
	$(CC) $(CCFLAGS) -I.. $(DTWOPLAYER) $(DSOCKET) -c game.c

Makefile.TC: ..\src-conf.mk tc.mk
	copy ..\src-conf.mk+tc.mk Makefile.TC

clean:
	del game.lib
	del game.bak
	del *.obj
	del Makefile.TC
