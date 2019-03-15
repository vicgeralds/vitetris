@echo off
if x%1==xlfn83 goto LFN83
if x%MAKE%==x set MAKE=make
if x%CC%==x set CC=tcc
%MAKE% -ftc.mk Makefile.TC
%MAKE% -fMakefile.TC %1
goto END
:LFN83
set lfn83_s_win=s_w
if exist curses~1.c set lfn83_s_win=s~1
if exist curse~60.c set lfn83_s_win=~60
>>lfn83.mk echo lfn83_s_win = %lfn83_s_win%
:END
