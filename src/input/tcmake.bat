@echo off
if x%1==xlfn83 goto LFN83
if x%MAKE%==x set MAKE=make
if x%CC%==x set CC=tcc
%MAKE% -ftc.mk Makefile.TC
%MAKE% -fMakefile.TC %1
goto END
:LFN83
set lfn83_urses=urs
if exist inp_cu~1.c set lfn83_urses=u~1
if exist inp_c~70.c set lfn83_urses=~70
>lfn83.mk echo lfn83_urses = %lfn83_urses%
:END
