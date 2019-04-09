CC = cc
CCFLAGS = $(CFLAGS) $(CPPFLAGS)
#CFLAGS = -O2 -Wall -pedantic -Wno-parentheses
CFLAGS = -O2
DJOYSTICK = -DJOYSTICK=1
joylinux_obj = joylinux.o
select_obj = select.o
#BACKEND = curses
#DCURSES = -DCURSES=1
BACKEND = ansi
#BACKEND = allegro
#DALLEGRO = -DALLEGRO=1
DXLIB = -DXLIB=1
DTERM_RESIZING = -DTERM_RESIZING=1
net_lib = net.a
DSOCKET = -DSOCKET=1
DINET = -DINET=1
inet_obj = inet.o
DTTY_SOCKET = -DTTY_SOCKET=1
tty_socket_obj = tty_socket.o
DHISCORE_FILENAME = -D'HISCORE_FILENAME="/var/games/vitetris-hiscores"'
#DPCTIMER = -DPCTIMER=1
#pctimer_obj = pctimer.o
