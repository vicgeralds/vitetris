include config.mk

PROGNAME = tetris$(EXE)

# Uncomment to change the default.  (Only used in Unix-like systems.)
#HISCORE_FILENAME = /var/games/vitetris-hiscores

INSTALL = install -oroot -groot

default: build
	@echo Done.
	@echo 'Now run ./$(PROGNAME) (or make install)'

$(PROGNAME):
	$(MAKE) build

build: src/src-conf.mk
	cd src; $(MAKE) tetris
	mv -f src/tetris$(EXE) $(PROGNAME)
	@echo stripping symbols to reduce program size:
	-strip --strip-all $(PROGNAME)

gameserver: src/netw/gameserver.c
	cd src/netw; $(MAKE) gameserver
	mv -f src/netw/gameserver .

src/src-conf.mk: config.mk Makefile src-conf.sh
	@echo generating $@
	./src-conf.sh '$(CC)' '$(CFLAGS)' '$(CPPFLAGS)'
	./src-conf.sh def TWOPLAYER $(TWOPLAYER)
	./src-conf.sh obj tetris2p $(TWOPLAYER)
	./src-conf.sh def JOYSTICK $(JOYSTICK)
	./src-conf.sh obj joylinux $(JOYSTICK)
	./src-conf.sh obj select $(UNIX)
	./src-conf.sh set BACKEND curses $(CURSES)
	./src-conf.sh def CURSES $(CURSES)
	./src-conf.sh set CURSES_INC "$(CURSES_INC)" $(CURSES)
	./src-conf.sh set BACKEND ansi -z $(CURSES)$(ALLEGRO)
	./src-conf.sh set BACKEND allegro $(ALLEGRO)
	./src-conf.sh def ALLEGRO $(ALLEGRO)
	./src-conf.sh def XLIB $(XLIB)
	./src-conf.sh def TERM_RESIZING $(TERM_RESIZING)
	./src-conf.sh def NO_MENU -z $(MENU)
	./src-conf.sh lib menuext $(MENU)
	./src-conf.sh def NO_BLOCKSTYLES -z $(BLOCKSTYLES)
	./src-conf.sh lib netw $(NETWORK)
	./src-conf.sh def SOCKET $(NETWORK)
	./src-conf.sh def INET $(NETWORK)
	./src-conf.sh obj inet $(NETWORK)
	./src-conf.sh def TTY_SOCKET "$(NETWORK)" -a $(TTY_SOCKET)
	./src-conf.sh obj tty_socket "$(NETWORK)" -a $(TTY_SOCKET)
	./src-conf.sh set DHISCORE_FILENAME "-D'HISCORE_FILENAME=\"$(HISCORE_FILENAME)\"'" $(HISCORE_FILENAME)
	./src-conf.sh def PCTIMER $(PCTIMER)
	./src-conf.sh obj pctimer $(PCTIMER)

install: $(PROGNAME)
	$(INSTALL) -d $(DESTDIR)$(bindir) $(DESTDIR)$(docdir)
	$(INSTALL) -m755 $(PROGNAME) $(DESTDIR)$(bindir)
	$(INSTALL) -m644 README licence.txt $(DESTDIR)$(docdir)
	if [ -n "$(pixmapdir)" ]; then \
  $(INSTALL) -d $(DESTDIR)$(pixmapdir) && \
  $(INSTALL) -m644 vitetris.xpm $(DESTDIR)$(pixmapdir); fi
	if [ -n "$(desktopdir)" ]; then \
  $(INSTALL) -d $(DESTDIR)$(desktopdir) && \
  $(INSTALL) -m644 vitetris.desktop $(DESTDIR)$(desktopdir); fi
	if [ -n "$(ALLEGRO)" ]; then \
  $(INSTALL) -d $(DESTDIR)$(datadir) && \
  $(INSTALL) -m644 pc8x16.fnt $(DESTDIR)$(datadir); fi
	@echo Done.
	@echo You may also wish to create the system-wide highscore file
	@echo 'with "make install-hiscores"'.

install-hiscores:
	@HS_FN=$(HISCORE_FILENAME); \
	if [ -z "$$HS_FN" ]; then HS_FN=/var/games/vitetris-hiscores; fi; \
	HS_FN="$(DESTDIR)$$HS_FN"; \
	echo $(INSTALL) -d $${HS_FN%/*};  $(INSTALL) -d "$${HS_FN%/*}" && \
	echo touch $$HS_FN &&             touch "$$HS_FN" && \
	echo chgrp games $$HS_FN &&       chgrp games "$$HS_FN" && \
	echo chmod g+w $$HS_FN &&         chmod g+w "$$HS_FN"

uninstall:
	rm -f $(bindir)/$(PROGNAME)
	rm -f $(docdir)/README
	rm -f $(docdir)/licence.txt
	rmdir $(docdir)
	test -z "$(pixmapdir)" || rm -f $(pixmapdir)/vitetris.xpm
	-rmdir "$(pixmapdir)"
	test -z "$(desktopdir)" || rm -f $(desktopdir)/vitetris.desktop
	-rmdir "$(desktopdir)"
	-rm -f $(datadir)/pc8x16.fnt
	-rmdir $(datadir)
clean:
	rm -f systest systest.exe
	cd src; $(MAKE) clean

.PHONY: default build install install-hiscores uninstall clean
