# DOS.MK
# Makefile for MS-DOS.  It is used to generate src\src-conf.mk.
# Don't expect anything to work automatically if you're using something other
# than DJGPP, Turbo C 2.01 or Turbo C++ 1.01.

build:
	dosmake tetris $(CC)

conf2.bat: config.mk dos.mk conf1.bat src-conf.bat
	del src\src-conf.mk
	conf1 begin
	conf1 $(CC) $(CFLAGS) $(CPPFLAGS) $(CURSES_INC)
	conf1 def TWOPLAYER $(TWOPLAYER)
	conf1 obj tetris2p $(TWOPLAYER)
	conf1 def JOYSTICK $(JOYSTICK)
	conf1 set BACKEND curses $(CURSES)
	conf1 def CURSES $(CURSES)
	conf1 set BACKEND ansi -z $(CURSES)$(ALLEGRO)
	conf1 set BACKEND allegro $(ALLEGRO)
	conf1 def ALLEGRO $(ALLEGRO)
	conf1 def NO_MENU -z $(MENU)
	conf1 lib menuext $(MENU)
	conf1 def NO_BLOCKSTYLES -z $(BLOCKSTYLES)
	conf1 def PCTIMER $(PCTIMER)
	conf1 obj pctimer $(PCTIMER)

Makefile.DOS: config.mk dos.mk
	copy config.mk+dos.mk Makefile.DOS

clean:
	del Makefile.DOS
	del systest.exe
	del systest.obj
	cd src
	tcmake clean
	cd ..
