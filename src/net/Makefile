include ../src-conf.mk

OBJS = socket.o comm.o $(inet_obj) $(tty_socket_obj)
config_h = ../config.h ../config2.h

all: $(net_lib)$(inet_obj)

net.ainet.o: $(OBJS) comm_inet.o
	-ar dc net.a no_inet.o
	ar rs net.a $(OBJS) comm_inet.o

net.a: $(OBJS) no_inet.o ../src-conf.mk
	-ar dc net.a inet.o comm_inet.o
	ar rs net.a $(OBJS) no_inet.o

socket.o: socket.c sock.h internal.h ../input/input.h $(config_h)

comm.o: comm.c sock.h ../game/tetris.h ../game/tetris2p.h \
		      ../input/input.h ../draw.h

comm_inet.o: comm_inet.c sock.h ../game/tetris.h

inet.o: inet.c sock.h internal.h $(config_h)

tty_socket.o: tty_socket.c sock.h internal.h
	$(CC) $(CCFLAGS)      -c tty_socket.c

no_inet.o: no_inet.c sock.h
	$(CC) $(CCFLAGS)      -c no_inet.c

.c.o:
	$(CC) $(CCFLAGS) -I.. -c $<

gameserver: gameserver.c
	$(CC) $(CCFLAGS) -o gameserver gameserver.c

clean:
	rm -f net.a socket.o comm.o comm_inet.o inet.o tty_socket.o no_inet.o
	rm -f gameserver

.PHONY: all net.ainet.o clean
