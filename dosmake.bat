@echo off
if x%MAKE%==x set MAKE=make
%MAKE% -fdos.mk Makefile.DOS
%MAKE% -fMakefile.DOS conf2.bat
if not exist src\src-conf.mk call conf2.bat
if x%1==xtetris goto TETRIS
%MAKE% -fMakefile.DOS %1
goto END
:TETRIS
cd src
if x%2==x goto T
if x%2==xtcc goto T
if x%2==xTCC goto T
if x%2==xtcc.exe goto T
if x%2==xTCC.EXE goto T
%MAKE% tetris
cd ..
if not exist src\tetris.exe goto END
move /y src\tetris.exe tetris.exe
strip --strip-all tetris.exe
goto END
:T
call tcmake.bat
cd ..
if not exist src\tetris.exe goto END
move /y src\tetris.exe tetris.exe
:END
