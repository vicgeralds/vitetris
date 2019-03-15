@echo off
if x%1==xlfn83 goto LFN83
if x%MAKE%==x set MAKE=make
if x%CC%==x set CC=tcc
%MAKE% -ftc.mk Makefile.TC
%MAKE% -fMakefile.TC %1
goto END
:LFN83
set lfn83_menu=men
set lfn83_p_menu=p_m
if exist draw_m~1.c set lfn83_menu=m~1
if exist draw_~4s.c set lfn83_menu=~4s
if exist draw2p~1.c set lfn83_p_menu=p~1
if exist draw2~z2.c set lfn83_p_menu=~z2
>lfn83.mk  echo lfn83_menu = %lfn83_menu%
>>lfn83.mk echo lfn83_p_menu = %lfn83_p_menu%
:END
