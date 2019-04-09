include ../src-conf.mk

OBJS = tetris.o tetris2p.o game.o wonlost.o
HEADERS = tetris.h ../timer.h ../input/input.h ../draw.h

game.a: $(OBJS)
	ar rs game.a $(OBJS)

tetris.o: tetris.c $(HEADERS)
	$(CC) $(CCFLAGS) -I..  -c tetris.c

tetris2p.o: tetris2p.c tetris2p.h $(HEADERS) game.h ../net/sock.h ../src-conf.mk
	$(CC) $(CCFLAGS) -I.. $(DSOCKET) -c tetris2p.c

game.o: game.c game.h tetris.h tetris2p.h ../options.h ../input/input.h \
	../net/sock.h ../hiscore.h ../menu/menu.h ../src-conf.mk
	$(CC) $(CCFLAGS) -I.. $(DJOYSTICK) $(DSOCKET) -c game.c

wonlost.o: wonlost.c game.h ../options.h
	$(CC) $(CCFLAGS) -c wonlost.c

clean:
	rm -f game.a tetris.o tetris2p.o game.o wonlost.o

.PHONY: clean
