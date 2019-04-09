include config.mk

PROGNAME = tetris$(EXE)

# Global highscore file (only used ifdef UNIX)
# No hardcoded filename will be used if commented out
HISCORE_FILENAME = /var/games/vitetris-hiscores

INSTALL = install -oroot -groot
INSTALL_DIR  = $(INSTALL) -d
INSTALL_PROG = $(INSTALL) -m755
INSTALL_DATA = $(INSTALL) -m644

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

gameserver: src/net/gameserver.c
	cd src/net; $(MAKE) gameserver
	mv -f src/net/gameserver .

src/src-conf.mk: config.mk Makefile src-conf.sh
	@echo generating $@
	./src-conf.sh '$(CC)' '$(CFLAGS)' '$(CPPFLAGS)'
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
	./src-conf.sh lib net $(NETWORK)
	./src-conf.sh def SOCKET $(NETWORK)
	./src-conf.sh def INET $(NETWORK)
	./src-conf.sh obj inet $(NETWORK)
	./src-conf.sh def TTY_SOCKET "$(NETWORK)" -a $(TTY_SOCKET)
	./src-conf.sh obj tty_socket "$(NETWORK)" -a $(TTY_SOCKET)
	./src-conf.sh set DHISCORE_FILENAME "-D'HISCORE_FILENAME=\"$(HISCORE_FILENAME)\"'" $(HISCORE_FILENAME)
	./src-conf.sh def PCTIMER $(PCTIMER)
	./src-conf.sh obj pctimer $(PCTIMER)

install: $(PROGNAME)
	$(INSTALL_DIR) "$(DESTDIR)$(bindir)" "$(DESTDIR)$(docdir)"
	$(INSTALL_PROG) $(PROGNAME) "$(DESTDIR)$(bindir)"
	$(INSTALL_DATA) README licence.txt "$(DESTDIR)$(docdir)"
	if [ -n "$(pixmapdir)" ]; then \
  $(INSTALL_DIR) "$(DESTDIR)$(pixmapdir)" && \
  $(INSTALL_DATA) vitetris.xpm "$(DESTDIR)$(pixmapdir)"; fi
	if [ -n "$(desktopdir)" ]; then \
  $(INSTALL_DIR) "$(DESTDIR)$(desktopdir)" && \
  $(INSTALL_DATA) vitetris.desktop "$(DESTDIR)$(desktopdir)"; fi
	if [ -n "$(ALLEGRO)" ]; then \
  $(INSTALL_DIR) "$(DESTDIR)$(datadir)" && \
  $(INSTALL_DATA) pc8x16.fnt "$(DESTDIR)$(datadir)"; fi
	@echo Done.
	@echo You may also wish to create the system-wide highscore file
	@echo 'with "make install-hiscores"'.

install-hiscores:
	@HS_FN="$(DESTDIR)$(HISCORE_FILENAME)"; \
	echo $(INSTALL_DIR) $${HS_FN%/*}; \
	$(INSTALL_DIR) $${HS_FN%/*}
	touch "$(DESTDIR)$(HISCORE_FILENAME)"
	chgrp games "$(DESTDIR)$(HISCORE_FILENAME)"
	chmod g+w "$(DESTDIR)$(HISCORE_FILENAME)"

uninstall:
	rm -f "$(bindir)/$(PROGNAME)"
	rm -f "$(docdir)/README"
	rm -f "$(docdir)/licence.txt"
	rmdir "$(docdir)"
	test -z "$(pixmapdir)" || rm -f "$(pixmapdir)/vitetris.xpm"
	-rmdir "$(pixmapdir)"
	test -z "$(desktopdir)" || rm -f "$(desktopdir)/vitetris.desktop"
	-rmdir "$(desktopdir)"
	-rm -f "$(datadir)/pc8x16.fnt"
	-rmdir "$(datadir)"
clean:
	rm -f systest systest.exe
	cd src; $(MAKE) clean

.PHONY: default build install install-hiscores uninstall clean
