@echo off 
rem generate src\src-conf.mk
if x%1==xdef goto DEF
if x%1==xobj goto OBJ
if x%1==xlib goto LIB
if x%1==xset goto SET
>src\src-conf.mk echo # src\src-conf.mk
if not x%1==x >>src\src-conf.mk echo CC = %1
if not x%3==x >>src\src-conf.mk echo CPPFLAGS = %3
if x%2==x     >>src\src-conf.mk echo CFLAGS = -O2
if not x%2==x >>src\src-conf.mk echo CFLAGS = %2
>>src\src-conf.mk echo #CFLAGS = -O2 -Wall -pedantic -Wno-parentheses
>>src\src-conf.mk echo CCFLAGS = $(CFLAGS) $(CPPFLAGS)
if x%1==xgcc goto G
if x%1==xtcc goto T
if x%1==xTCC goto T
if x%1==xtcc.exe goto T
if x%1==xTCC.EXE goto T
if x%DJGPP%==x goto T
:G
>>src\src-conf.mk echo OBJEXT = .o
>>src\src-conf.mk echo LIBEXT = .a
goto END
:T
>>src\src-conf.mk echo OBJEXT = .obj
>>src\src-conf.mk echo LIBEXT = .lib
>>src\src-conf.mk echo LIBEXE = tlib
goto END
:DEF
set L=D%2
set R=-D
goto A
:OBJ
set L=%2_obj
set R=%2$(OBJEXT)
goto A
:LIB
set L=%2_lib
set R=%2$(LIBEXT)
goto A
:SET
set L=%2
set R=%3
shift
:A
if x%2%3==x goto END
if not x%3==x-z goto B
if not x%4==x goto C
:B
if x%3==x goto C
if not x%R%==x-D >>src\src-conf.mk echo %L% = %R%
if x%R%==x-D     >>src\src-conf.mk echo %L% = -%L%=1
goto END
:C
if not x%R%==x-D >>src\src-conf.mk echo #%L% = %R%
if x%R%==x-D     >>src\src-conf.mk echo #%L% = -%L%=1
:END
set L=
set R=
