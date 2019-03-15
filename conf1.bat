@echo off 
if not x%1==xbegin goto L
>conf2.bat echo @echo off
goto E
:L
>conf2.tmp echo call src-conf.bat %1 %2 %3 %4 %5
copy conf2.bat+conf2.tmp conf2.bat > nul
del conf2.tmp
:E
