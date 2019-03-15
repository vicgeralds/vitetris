@echo off
if x%1==xlfn83 goto LFN83
if x%MAKE%==x set MAKE=make
if x%CC%==x set CC=tcc
%MAKE% -ftc.mk Makefile.TC
%MAKE% -fMakefile.TC %1
goto END
:LFN83
set lfn83_style=sty
set lfn83_urses=urs
if exist blocks~1.c set lfn83_style=s~1
if exist block~wv.c set lfn83_style=~wv
if exist men_cu~1.c set lfn83_urses=u~1
if exist men_c~9i.c set lfn83_urses=~9i
>lfn83.mk  echo lfn83_style = %lfn83_style%
>>lfn83.mk echo lfn83_urses = %lfn83_urses%
:END
