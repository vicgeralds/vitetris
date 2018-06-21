include ../src-conf.mk

OBJS = draw.o draw_menu.o draw2p.o draw2p_menu.o
HEADERS = draw.h internal.h ../textgfx/textgfx.h

draw.a: $(OBJS)
	ar rs draw.a $(OBJS)

draw.o: draw.c $(HEADERS) ../game/tetris.h ../hiscore.h \
			  ../netw/sock.h ../src-conf.mk
	$(CC) $(CCFLAGS) -I.. $(DTWOPLAYER) $(DSOCKET) -c draw.c

draw_menu.o: draw_menu.c $(HEADERS) ../version.h
	$(CC) $(CCFLAGS)      -c draw_menu.c

draw2p.o: draw2p.c $(HEADERS) ../game/tetris.h
	$(CC) $(CCFLAGS) -I.. -c draw2p.c

draw2p_menu.o: draw2p_menu.c $(HEADERS)
	$(CC) $(CCFLAGS)      -c draw2p_menu.c

clean:
	rm -f draw.a $(OBJS)
	rm -f draw_~4s.o draw2~z2.o

.PHONY: clean
