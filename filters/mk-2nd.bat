@echo off
rem $Header: /users/source/archives/vile.vcs/filters/RCS/mk-2nd.bat,v 1.5 2000/08/16 10:02:32 tom Exp $
rem like mk-2nd.awk, used to generate rules from genmake.mak

goto %2

:compile_c
genmake.exe -n "%4$o :"
genmake.exe -n "	$(cc) -Dfilter_def=define_%3 $(CFLAGS) -c %4.c -Fo$@"
genmake.exe -n ""
goto done

:compile_l
rem The odd "LEX.%3_.c" happens to be what flex generates.
genmake.exe -n "%4$o :"
genmake.exe -n "	$(LEX) -P%3_ %4.l"
genmake.exe -n "	$(CC) -Dfilter_def=define_%3 $(CFLAGS) -c LEX.%3_.c -Fo$@"
genmake.exe -n "	- erase LEX.%3_.c"
genmake.exe -n ""
goto done

:link_c
genmake.exe -n "vile-%3-filt$x : %4$o $(CF_DEPS)"
genmake.exe -n "	$(link) -out:$@ $(CON_LDFLAGS) %4$o $(CF_ARGS)"
genmake.exe -n ""
goto done

:link_l
genmake.exe -n "vile-%3-filt$x : %4$o $(LF_DEPS)"
genmake.exe -n "	$(link) -out:$@ $(CON_LDFLAGS) %4$o $(LF_ARGS)
genmake.exe -n ""
goto done

:extern
genmake.exe -n "# generated by %0.bat"
genmake.exe -x%0 ". link_%%k %%i %%j %%k" <%1
goto done

:intern
genmake.exe -n "# generated by %0.bat"
rem library rule is in makefile.wnt, since we cannot echo redirection
rem chars needed for "inline" (aka here-document).
genmake.exe -x%0 ". compile_%%k %%i %%j %%k" <%1
goto done

:done