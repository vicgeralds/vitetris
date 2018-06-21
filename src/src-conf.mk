CCFLAGS = $(CFLAGS) $(CPPFLAGS)
#CFLAGS = -O2 -Wall -pedantic -Wno-parentheses
CFLAGS = -O2
DTWOPLAYER = -DTWOPLAYER=1
tetris2p_obj = tetris2p.o
#DJOYSTICK = -DJOYSTICK=1
#joylinux_obj = joylinux.o
select_obj = select.o
#BACKEND = curses
#DCURSES = -DCURSES=1
BACKEND = ansi
#BACKEND = allegro
#DALLEGRO = -DALLEGRO=1
#DXLIB = -DXLIB=1
DTERM_RESIZING = -DTERM_RESIZING=1
#DNO_MENU = -DNO_MENU=1
menuext_lib = menuext.a
#DNO_BLOCKSTYLES = -DNO_BLOCKSTYLES=1
netw_lib = netw.a
DSOCKET = -DSOCKET=1
DINET = -DINET=1
inet_obj = inet.o
DTTY_SOCKET = -DTTY_SOCKET=1
tty_socket_obj = tty_socket.o
