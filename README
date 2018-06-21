VITETRIS - Tetris clone by Victor Nilsson

  Homepage: http://victornils.net/tetris

  Rotation, scoring, levels and speed should resemble the early Tetris games
  by Nintendo, with the addition of a short lock delay which makes it
  possible to play at higher levels.  (It does not make it possible to
  prevent the piece from ever locking by abusing lock delay resets.)

  vitetris comes with more features and options than might be expected from
  a simple text mode game.  Full input control, customizable appearance,
  netplay where both players can choose difficulty (level and height) --
  unless you must have sound (or just don't like Tetris), you won't be
  dissappointed.  Everything is not described here, but you should be able
  to figure it out.

  This program is free software.  It is distributed under the terms of a
  simple BSD-style licence (OSI-approved).  See licence.txt for details.
  

How to Play
-----------

  Run "./tetris" inside a terminal window if you have just extracted a
  tarball somewhere (cd into the directory first).  The command could be
  "vitetris" or "tetris" if you have used a package manager to install it.
  (If you're on Windows, just double-click tetris.exe.)

  To navigate in the menus, use the cursor keys, ENTER to proceed (or leave
  a menu), and BACKSPACE or ESC to go back.  Exit at once with Q.

  Default Game Controls

	Move left:  LEFT
	Move right: RIGHT
	Rotate:	    UP
	Softdrop:   DOWN
	Harddrop:   SPACE

  To pause in a single-player game, press P or ENTER.

  A-type means a normal "marathon" game.
  In a B-type game, the object is to clear 25 lines.


Options
-------

  The first thing you want to do if you're in a terminal window with a white
  background is to enter "Options" and set "Term BG" to "white" (press
  right).  Otherwise you'll get a white/grey piece which is hard to see.
  You can also change the colours of individual pieces in the "Tetromino
  Colours" menu.

  To change key bindings, go to "Input Setup".  To the left are keys used in
  menus, to the right in-game keys.  Just press the key you want to use.

  If not explicitly set, the following in-game keys depend on some other key
  binding (e.g. if UP is used for "Up", it is also used for "Rotate"):

	 Rotate  - Up
	 Rot cw  - A
	 Rot acw - B
	Softdrop - Dwn

  "Rot cw" means "Rotate clockwise" and "acw" anticlockwise.

  There are three separate configurations, single player, player1 and
  player2, which include keys, starting level and height, and rotation
  preference.  In netplay, the single player configuration is used.


The Command Line Is Your Friend
-------------------------------

  Actually, you can do pretty much everything from the menus, even netplay
  (since version 0.54) if you only want to connect to a server.  But still,
  you will always get a little more from the command line.

  Use "tetris -help" (or whatever the command name is) to get a help message
  with a list of command-line options.  If you're on Windows, you need to
  run this from a cmd window, or a "DOS window".

  This help message tells you where configuration and highscores are saved.


Network Play
------------

  The easiest way to do netplay is to connect to a vitetris game server from
  the "Netplay" menu.  Just type the address of the server and press ENTER.
  If there are other players connected and available, they will be listed.
  You will not appear as available to others until "Waiting for opponent..."
  is displayed.

  Check http://victornils.net/tetris/#servers for public servers.

  My game server is included in the source distribution.  The old one
  described below is also included, if you want something really simple.


Network Play: The Traditional Way
---------------------------------

  To play against someone on the internet, one needs to listen with

	tetris listen PORT

  (where PORT is a number, e.g. 34034); the other connects with

	tetris connect HOSTNAME:PORT

  HOSTNAME may be an IP address or a hostname.  If it is omitted, localhost
  is used.

  If the server (the one who listens) has mode set to B-type, such a game
  will be set up.

  It is also possible to play using Unix domain sockets, which means that
  filenames on the local system are used as addresses.  The most convenient
  way to set up such a connection is as follows.  Start tetris on two
  different terminals.  Then one player enters "2-Player Game", which will
  bring up a list of ttys.  When the tty of the other player is selected, an
  invitation will appear on his screen.

  I have written a simple game server which is included in the source 
  distribution.  Both players connect to this server, which means that
  there's no need to agree on who's to listen and who's to connect, and it
  works behind firewalls.  More than two players can be connected at the same
  time, so one could make random connects if there are many active players.


Gamepad Start Button
--------------------

  The START button on a gamepad can be mapped to act like the ENTER key.
  Add the following line below [js0] or [js1] in .vitetris (or vitetris.cfg):

	start=NUM

  where NUM is the button number.


Different Versions, Library Dependencies
----------------------------------------

  The original idea was to make a simple Tetris with no library dependencies
  other than libc, which would compile in one second on any Linux system.
  (Btw, one big motivation was that I couldn't find any good Tetris clone 
  for Linux which suited my taste.)

  Then some Xlib code was added, to detect lost window focus (and terminal
  resize, before I knew about SIGWINCH).  If you have a binary which is
  linked against libX11 (find out with ldd), it will not run on a system
  without X.

  vitetris can use a curses library (ncurses, PDCurses) to do cursor
  movement, set display attributes, read keys, etc.  There is also a version
  for Allegro (a game library), which displays text in graphics mode using a
  VGA font.
 

Credits
-------

  The original Tetris game was designed and programmed by Alexey Pajitnov in
  1985.

  The wiki at tetrisconcept.com has been a valuable resource.

  tt "Tetris for Terminals" was written by Mike Taylor in 1989.

  Hugo Fernbom and Andreas Carlsson have helped me with testing, ideas and
  feedback.
