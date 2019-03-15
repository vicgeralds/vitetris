@echo off
call src-conf.bat gcc    
call src-conf.bat def TWOPLAYER y  
call src-conf.bat obj tetris2p y  
call src-conf.bat def JOYSTICK   
call src-conf.bat set BACKEND curses y 
call src-conf.bat def CURSES y  
call src-conf.bat set BACKEND ansi -z y
call src-conf.bat set BACKEND allegro  
call src-conf.bat def ALLEGRO   
call src-conf.bat def NO_MENU -z y 
call src-conf.bat lib menuext y  
call src-conf.bat def NO_BLOCKSTYLES -z y 
call src-conf.bat def PCTIMER   
call src-conf.bat obj pctimer   
