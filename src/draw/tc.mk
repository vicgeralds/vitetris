#
# src\draw\tc.mk

draw_menu = draw_$(lfn83_menu)
draw2p_menu = draw2$(lfn83_p_menu)

OBJS = draw.obj $(draw_menu).obj draw2p.obj $(draw2p_menu).obj
HEADERS = draw.h internal.h ..\textgfx\textgfx.h

draw.lib: $(OBJS)
	del draw.lib
	$(LIBEXE) draw.lib +draw.obj +$(draw_menu).obj +draw2p.obj +$(draw2p_menu).obj

draw.obj: draw.c $(HEADERS) ..\game\tetris.h ..\hiscore.h Makefile.TC
	$(CC) $(CCFLAGS) -I.. $(DTWOPLAYER) -c draw.c

$(draw_menu).obj: $(draw_menu).c $(HEADERS) ..\version.h
	$(CC) $(CCFLAGS) -c $(draw_menu).c

draw2p.obj: draw2p.c $(HEADERS) ..\game\tetris.h
	$(CC) $(CCFLAGS) -I.. -c draw2p.c

$(draw2p_menu).obj: $(draw2p_menu).c $(HEADERS)
	$(CC) $(CCFLAGS) -c $(draw2p_menu).c

Makefile.TC: ..\src-conf.mk lfn83.mk tc.mk
	copy ..\src-conf.mk+lfn83.mk+tc.mk Makefile.TC

lfn83.mk: tcmake.bat
	tcmake lfn83

clean:
	del draw.lib
	del *.obj
	del Makefile.TC
	del lfn83.mk
