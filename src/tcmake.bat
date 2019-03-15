@echo off
if x%1==xlfn83 goto LFN83
if x%1==xlibs  goto LIBS
if x%MAKE%==x set MAKE=make
if x%CC%==x set CC=tcc
%MAKE% -ftc.mk Makefile.TC
%MAKE% -fMakefile.TC %1
goto END
:LFN83
set lfn83_ne_empty=ne-
if exist cmdlin~1.c set lfn83_ne_empty=n~1
if exist cmdli~f7.c set lfn83_ne_empty=~f7
>lfn83.mk echo lfn83_ne_empty = %lfn83_ne_empty%
goto END
:LIBS
cd game
call tcmake.bat
if errorlevel 1 goto END
if exist game.lib goto cdmenu
echo ERROR! could not make game.lib
goto END
:cdmenu
cd ..\menu
call tcmake.bat
if errorlevel 1 goto END
if exist menu.lib goto cdinput
echo ERROR! could not make menu.lib
goto END
:cdinput
cd ..\input
call tcmake.bat %2
if errorlevel 1 goto END
if exist input.lib goto cddraw
echo ERROR! could not make input.lib
goto END
:cddraw
cd ..\draw
call tcmake.bat
if errorlevel 1 goto END
if exist draw.lib goto cdtextgfx
echo ERROR! could not make draw.lib
goto END
:cdtextgfx
cd ..\textgfx
call tcmake.bat
if errorlevel 1 goto END
if exist textgfx.lib goto cddotdot
echo ERROR! could not make textgfx.lib
goto END
:cddotdot
cd ..
move /y game\game.lib .
move /y menu\menu.lib .
if exist menu\menuext.lib move /y menu\menuext.lib .
move /y input\input.lib .
move /y draw\draw.lib .
move /y textgfx\textgfx.lib .
:END
