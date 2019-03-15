@echo off
if x%MAKE%==x set MAKE=make
if x%CC%==x set CC=tcc
%MAKE% -ftc.mk Makefile.TC
%MAKE% -fMakefile.TC %1
