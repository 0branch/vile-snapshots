/*
 * vile -- "vi like emacs"
 *
 * this used to be MicroEMACS, a public domain program written by
 * dave g. conroy, further improved and modified by daniel m. lawrence.
 *
 * the original author of vile is paul fox.  tom dickey and kevin
 * buettner made huge contributions along the way, as did rick
 * sladkey and other people (see all of the CHANGES files for
 * details).  vile is now principally maintained by tom dickey.
 *
 * previous versions of vile were limited to non-commercial use due to
 * their inclusion of code from versions of MicroEMACS which were
 * restricted in that way.  in the current version of vile, every
 * attempt has been made to ensure that this later code has been
 * removed or rewritten, returning vile's original basis to freely
 * distributable status.  This version of vile is distributed under the
 * terms of the GNU Public License (see COPYING).
 *
 * Copyright (c) 1992-2002 by Paul Fox and Thomas Dickey
 *
 */

/*
 * $Header: /users/source/archives/vile.vcs/RCS/main.c,v 1.499 2003/03/08 15:51:32 tom Exp $
 */

#define realdef			/* Make global definitions not external */

#include	"estruct.h"	/* global structures and defines */
#include	"edef.h"	/* global declarations */
#include	"nevars.h"
#include	"nefunc.h"
#include	"nefsms.h"

#include	<assert.h>

#if OPT_LOCALE
#include	<locale.h>

#ifdef HAVE_WCTYPE
#include	<wctype.h>
#define sys_iscntrl(n)  iswcntrl(n)
#define sys_isdigit(n)  iswdigit(n)
#define sys_islower(n)  iswlower(n)
#define sys_isprint(n)  iswprint(n)
#define sys_ispunct(n)  iswpunct(n)
#define sys_isspace(n)  iswspace(n)
#define sys_isupper(n)  iswupper(n)
#define sys_isxdigit(n) iswxdigit(n)
#else
#include	<ctype.h>
#define sys_iscntrl(n)  iscntrl(n)
#define sys_isdigit(n)  isdigit(n)
#define sys_islower(n)  islower(n)
#define sys_isprint(n)  isprint(n)
#define sys_ispunct(n)  ispunct(n)
#define sys_isspace(n)  isspace(n)
#define sys_isupper(n)  isupper(n)
#define sys_isxdigit(n) isxdigit(n)
#endif

#endif /* OPT_LOCALE */

#if CC_NEWDOSCC
#include <io.h>
#if CC_DJGPP
#include <dpmi.h>
#include <go32.h>
#endif
#endif

#if SYS_VMS
#include <processes.h>
#endif

extern char *exec_pathname;
extern const char *const pathname[];	/* startup file path/name array */

/* if on a crippled system, try to survive: bigger stack */
#if SYS_MSDOS && CC_TURBO
unsigned _stklen = 24000U;
#endif

static int cmd_mouse_motion(const CMDFUNC * cfp);
static void get_executable_dir(void);
static void global_val_init(void);
static void main_loop(void);
static void make_startup_file(char *name);
static void siginit(void);
static void start_debug_log(int ac, char **av);

extern const int nametblsize;

/*--------------------------------------------------------------------------*/

/*
 * Get the argument parameter.  The 'param' points to the option flag.  Set it
 * to a 1-character string to force the next entry from argv[] to be fetched.
 */
static char *
get_argvalue(char *param, char *argv[], int *argcp)
{
    if (!*(++param)) {
	*argcp += 1;
	param = argv[*argcp];
    }
    if (param == 0)
	print_usage();
    return param;
}

#define	GetArgVal(param)	get_argvalue(param, argv, &carg)

/*--------------------------------------------------------------------------*/

int
MainProgram(int argc, char *argv[])
{
    int tt_opened;
    BUFFER *bp;
    int carg;			/* current arg to scan */
    char *vileinit = NULL;	/* the startup file or VILEINIT var */
    int startstat = TRUE;	/* result of running startup */
    BUFFER *havebp = NULL;	/* initial buffer to read */
    char *havename = NULL;	/* name of first buffer in cmd line */
    int gotoflag = FALSE;	/* do we need to goto line at start? */
    int gline = FALSE;		/* if so, what line? */
    int helpflag = FALSE;	/* do we need help at start? */
    REGEXVAL *search_exp = 0;	/* initial search-pattern */
    const char *msg;
#if SYS_VMS
    char *init_descrip = NULL;
#endif
#ifdef VILE_OLE
    int ole_register = FALSE;
#endif
#if OPT_TAGS
    int didtag = FALSE;		/* look up a tag to start? */
    char *tname = NULL;
#endif
#if DISP_NTCONS
    int new_console = FALSE;
#endif
#if OPT_ENCRYPT
    char startkey[NKEYLEN];	/* initial encryption key */
    *startkey = EOS;
#endif

#if OPT_LOCALE
    {
	char *env = "";

	/*
	 * Force 8-bit locale for display drivers where we only support 8-bits
	 */
#if DISP_TERMCAP
	if (((env = getenv("LC_ALL")) != 0 && *env != 0) ||
	    ((env = getenv("LC_CTYPE")) != 0 && *env != 0) ||
	    ((env = getenv("LANG")) != 0 && *env != 0)) {
	    char *utf = strstr(env, ".UTF-8");

	    if (utf != 0) {
		char *tmp = strmalloc(env);
		tmp[utf - env] = EOS;
		env = tmp;
		utf8_locale = TRUE;
	    } else {
		env = "";
	    }
	} else {
	    env = "";
	}
#endif
	setlocale(LC_ALL, env);	/* set locale according to environment vars */
    }
#endif

#if OPT_NAMEBST
    build_namebst(nametbl, 0, nametblsize - 1);
#endif
    global_val_init();		/* global buffer values */
    charinit();			/* character types -- we need these early  */
    winit(FALSE);		/* command-line */
#if !SYS_UNIX
    expand_wild_args(&argc, &argv);
#endif
    prog_arg = argv[0];		/* this contains our only clue to exec-path */
#if SYS_MSDOS || SYS_OS2 || SYS_OS2_EMX || SYS_WINNT
    if (strchr(pathleaf(prog_arg), '.') == 0) {
	char *t = malloc(strlen(prog_arg) + 5);
	lsprintf(t, "%s.exe", prog_arg);
	prog_arg = t;
    }
#endif

    /*
     * Attempt to appease perl if we're running in a process which was setuid'd
     * or setgid'd.
     */
#if defined(HAVE_SETUID) && defined(HAVE_SETGID) && defined(HAVE_GETEGID) && defined(HAVE_GETEUID)
    setgid(getegid());
    setuid(geteuid());
#endif

    start_debug_log(argc, argv);

    get_executable_dir();

    if (strcmp(pathleaf(prog_arg), "view") == 0)
	set_global_b_val(MDREADONLY, TRUE);

#if DISP_X11
    if (argc != 2 || strcmp(argv[1], "-V") != 0) {
	if (x_preparse_args(&argc, &argv) != TRUE)
	    startstat = FALSE;
    }
#endif

    /*
     * Allow for I/O to the command-line before we initialize the screen
     * driver.
     *
     * FIXME: we only know how to do this for displays that open the
     * terminal in the same way for command-line and screen.
     */
    siginit();
#if OPT_DUMBTERM
    if (isatty(fileno(stdin))
	&& isatty(fileno(stdout))) {
	tt_opened = open_terminal(&dumb_term);
    } else
#endif
	tt_opened = open_terminal(&null_term);

    /* Parse the passed in parameters */
    for (carg = 1; carg < argc; ++carg) {
	char *param = argv[carg];

	/* evaluate switches */
	if (*param == '-') {
	    ++param;
#if DISP_IBMPC || DISP_BORLAND || SYS_VMS
	    /* if it's a digit, it's probably a screen
	       resolution */
	    if (isDigit(*param)) {
#if DISP_IBMPC || DISP_BORLAND
		current_res_name = param;
#else
		if (strcmp(param, "132") == 0)
		    init_descrip = "WIDE";
		else if (strcmp(param, "80") == 0)
		    init_descrip = "NORMAL";
		else
		    print_usage();
#endif
		continue;
	    } else
#endif /* DISP_IBMPC */
		switch (*param) {
#if DISP_NTCONS
		case 'c':
		    if (strcmp(param, "console") == 0) {
			/*
			 * start editor in a new console env if
			 * stdin is redirected.  if this option
			 * is not set when stdin is redirected,
			 * console vile's mouse features are
			 * unavailable (bug).
			 */
			new_console = TRUE;
		    } else
			print_usage();
		    break;
#endif /* DISP_NTCONS */
#if OPT_EVAL || OPT_DEBUGMACROS
		case 'D':
		    tracemacros = TRUE;
		    break;
#endif
		case 'e':	/* -e for Edit file */
		case 'E':
		    set_global_b_val(MDVIEW, FALSE);
		    break;
		case 'g':	/* -g for initial goto */
		case 'G':
		    gotoflag = TRUE;
		    param = GetArgVal(param);
		    gline = atoi(param);
		    break;
		case 'h':	/* -h for initial help */
		case 'H':
		    helpflag = TRUE;
		    break;

		case 'i':
		case 'I':
		    vileinit = "vileinit.rc";
		    /*
		     * If the user has no startup file, make a simple
		     * one that points to this one.
		     */
		    if (cfg_locate(vileinit, LOCATE_SOURCE) != 0
			&& cfg_locate(startup_file, LOCATE_SOURCE) == 0)
			make_startup_file(vileinit);
		    break;

#if OPT_ENCRYPT
		case 'k':	/* -k<key> for code key */
		case 'K':
		    param = GetArgVal(param);
		    vl_make_encrypt_key(startkey, param);
		    break;
#endif
#ifdef VILE_OLE
		case 'O':
		    if (param[1] == 'r')
			ole_register = TRUE;
		    else
			print_usage();
		    break;
#endif
		case 's':	/* -s <pattern> */
		case 'S':
		  dosearch:
		    param = GetArgVal(param);
		    search_exp = new_regexval(param,
					      global_b_val(MDMAGIC));
		    break;
#if OPT_TAGS
		case 't':	/* -t for initial tag lookup */
		case 'T':
		    param = GetArgVal(param);
		    tname = param;
		    break;
#endif
		case 'v':	/* -v is view mode */
		    set_global_b_val(MDVIEW, TRUE);
		    break;

		case 'R':	/* -R is readonly mode */
		    set_global_b_val(MDREADONLY, TRUE);
		    break;

		case 'V':
#if DISP_NTWIN
		    gui_version(prog_arg);
#else
		    (void) printf("%s\n", getversion());
#endif
		    tidy_exit(GOODEXIT);
		    /* NOTREACHED */

		case '?':
		    /* FALLTHRU */
		default:	/* unknown switch */
		    print_usage();
		}

	} else if (*param == '+') {	/* alternate form of -g */
	    if (*(++param) == '/') {
		int len = strlen(param);
		if (len > 0 && param[len - 1] == '/')
		    param[--len] = EOS;
		if (len == 0)
		    print_usage();
		goto dosearch;
	    }
	    gotoflag = TRUE;
	    gline = atoi(param);
	} else if (*param == '@') {
	    vileinit = ++param;
	} else if (*param != EOS) {

	    /* must be a filename */
#if OPT_ENCRYPT
	    cryptkey = (*startkey != EOS) ? startkey : 0;
#endif
	    /* set up a buffer for this file */
	    bp = getfile2bp(param, FALSE, TRUE);
	    if (bp) {
		bp->b_flag |= BFARGS;	/* treat this as an argument */
		make_current(bp);	/* pull it to the front */
		if (!havebp) {
		    havebp = bp;
		    havename = param;
		}
	    }
#if OPT_ENCRYPT
	    cryptkey = 0;
#endif
	}
    }
#ifdef VILE_OLE
    if (ole_register) {
	/*
	 * Now that all command line options have been successfully
	 * parsed, register the OLE automation server and exit.
	 */

	ntwinio_oleauto_reg();
	/* NOT REACHED */
    }
#endif

    /* if stdin isn't a terminal, assume the user is trying to pipe a
     * file into a buffer.
     */
#if SYS_UNIX || SYS_VMS || SYS_MSDOS || SYS_OS2 || SYS_WINNT
#if DISP_NTWIN
    if (stdin_data_available())
#else
    if (!isatty(fileno(stdin)))
#endif
    {

#if !SYS_WINNT
#if !DISP_X11
#if SYS_UNIX
	char *tty = "/dev/tty";
#else
	FILE *in;
	int fd;
#endif /* SYS_UNIX */
#endif /* DISP_X11 */
#endif /* !SYS_WINNT */
	BUFFER *lastbp = havebp;
	int nline = 0;

	bp = bfind(STDIN_BufName, BFARGS);
	make_current(bp);	/* pull it to the front */
	if (!havebp)
	    havebp = bp;
	ffp = fdopen(dup(fileno(stdin)), "r");
#if !DISP_X11
# if SYS_UNIX
# if defined(HAVE_TTYNAME) && !defined(HAVE_DEV_TTY)
	tty = ttyname(fileno(stderr));
# endif
	/*
	 * Note: On Linux, the low-level close/dup operation
	 * doesn't work, since something hangs, apparently
	 * because substituting the file descriptor doesn't communicate
	 * properly up to the stdio routines.
	 */
	if ((freopen(tty, "r", stdin)) == 0
	    || !isatty(fileno(stdin))) {
	    fprintf(stderr, "cannot open a terminal (%s)\n", tty);
	    tidy_exit(BADEXIT);
	}
#else
# if SYS_WINNT
#  if DISP_NTCONS
	if (new_console) {
	    if (FreeConsole()) {
		if (!AllocConsole()) {
		    fputs("unable to allocate new console\n", stderr);
		    tidy_exit(BADEXIT);
		}
	    } else {
		fputs("unable to release existing console\n", stderr);
		tidy_exit(BADEXIT);
	    }
	}

	/*
	 * The editor must reopen the console, not fd 0.  If the console
	 * is not reopened, the nt console I/O routines die immediately
	 * when attempting to fetch a STDIN handle.
	 */
	freopen("con", "r", stdin);
#  endif
# else
#  if SYS_VMS
	fd = open("tt:", O_RDONLY, S_IREAD);	/* or sys$command */
#  else				/* e.g., DOS-based systems */
	fd = fileno(stderr);	/* this normally cannot be redirected */
#  endif
	if ((fd >= 0)
	    && (close(0) >= 0)
	    && (fd = dup(fd)) == 0
	    && (in = fdopen(fd, "r")) != 0)
	    *stdin = *in;
#  endif			/* SYS_WINNT */
# endif				/* SYS_UNIX */
#endif /* DISP_X11 */

#if OPT_ENCRYPT
	if (*startkey != EOS) {
	    strcpy(bp->b_cryptkey, startkey);
	    make_local_b_val(bp, MDCRYPT);
	    set_b_val(bp, MDCRYPT, TRUE);
	}
#endif
	(void) slowreadf(bp, &nline);
	set_rdonly(bp, bp->b_fname, MDREADONLY);
	(void) ffclose();

	if (is_empty_buf(bp)) {
	    (void) zotbuf(bp);
	    curbp = havebp = lastbp;
	}
#if OPT_FINDERR
	else {
	    set_febuff(bp->b_bname);
	}
#endif
    }
#endif

    if (!tt_opened)
	siginit();
    (void) open_terminal((TERM *) 0);
    term.kopen();		/* open the keyboard */
    term.rev(FALSE);

    /*
     * The terminal driver sets default colors and $palette during the
     * open_terminal function.  Save that information as the 'default'
     * color scheme.  We have to do this before reading .vilerc
     */
#if OPT_COLOR_SCHEMES
    init_scheme();
#endif

    if (vtinit() != TRUE)	/* allocate display memory */
	tidy_exit(BADEXIT);

    winit(TRUE);		/* windows */

    /* this comes out to 70 on an 80 (or greater) column display */
    {
	register int fill;
	fill = (7 * term.cols) / 8;	/* must be done after vtinit() */
	if (fill > 70)
	    fill = 70;
	set_global_b_val(VAL_FILL, fill);
    }

    /* Create an unnamed buffer, so that the initialization-file will have
     * something to work on.  We don't pull in any of the command-line
     * filenames yet, because some of the initialization stuff has to be
     * triggered by switching buffers after reading the .vilerc file.
     *
     * If nothing modifies it, this buffer will be automatically removed
     * when we switch to the first file (i.e., havebp), because it is
     * empty (and presumably isn't named the same as an actual file).
     */
    bp = bfind(UNNAMED_BufName, 0);
    bp->b_active = TRUE;
#if OPT_DOSFILES
    /* an empty non-existent buffer defaults to line-style
       favored by the OS */
    make_local_b_val(bp, MDDOS);
    set_b_val(bp, MDDOS, CRLF_LINES);
#endif
    fix_cmode(bp, FALSE);
    swbuffer(bp);

    /* run the specified, or the system startup file here.
       if vileinit is set, it's the name of the user's
       command-line startup file, i.e. 'vile @mycmds'
     */
    if (vileinit && *vileinit) {
	if (do_source(vileinit, 1, FALSE) != TRUE) {
	    startstat = FALSE;
	    goto begin;
	}
	free(startup_file);
	startup_file = strmalloc(vileinit);
    } else {

	/* else vileinit is the contents of their VILEINIT variable */
	vileinit = getenv("VILEINIT");
	if (vileinit != NULL) {	/* set... */
	    BUFFER *vbp, *obp;
	    UINT oflags = 0;
	    if (*vileinit) {	/* ...and not null */
		/* mark as modified, to prevent
		 * undispbuff() from clobbering */
		obp = curbp;
		if (obp) {
		    oflags = obp->b_flag;
		    b_set_changed(obp);
		}

		if ((vbp = bfind(VILEINIT_BufName, BFEXEC)) == 0)
		    tidy_exit(BADEXIT);

		/* don't want swbuffer to try to read it */
		vbp->b_active = TRUE;
		swbuffer(vbp);
		b_set_scratch(vbp);
		bprintf("%s", vileinit);
		/* if we leave it scratch, swbuffer(obp)
		   may zot it, and we may zot it again */
		b_clr_scratch(vbp);
		set_rdonly(vbp, vbp->b_fname, MDVIEW);

		/* go execute it! */
		if (dobuf(vbp, 1) != TRUE) {
		    startstat = FALSE;
		    goto begin;
		}
		if (obp) {
		    swbuffer(obp);
		    obp->b_flag = oflags;
		}
		/* remove the now unneeded buffer */
		b_set_scratch(vbp);	/* make sure it will go */
		(void) zotbuf(vbp);
	    }
	} else {		/* find and run .vilerc */
	    if (do_source(startup_file, 1, TRUE) != TRUE) {
		startstat = FALSE;
		goto begin;
	    }
	}
    }

    /*
     * before reading, double-check, since a startup-script may
     * have removed the first buffer.
     */
    if (havebp && find_bp(havebp)) {
	if (find_bp(bp) && is_empty_buf(bp) && !b_is_changed(bp))
	    b_set_scratch(bp);	/* remove the unnamed-buffer */
	if (swbuffer(havebp) == TRUE) {
#if OPT_MAJORMODE && OPT_HOOKS
	    /*
	     * Partial fix in case we edit .vilerc (vile.rc) or
	     * filters.rc, or related files that initialize syntax
	     * highlighting.
	     */
	    VALARGS args;
	    infer_majormode(curbp);
	    if (find_mode(curbp, "vilemode", FALSE, &args) == TRUE)
		run_readhook();
#else
	    ;
#endif
	} else {
	    startstat = FALSE;
	}
	if (havename)
	    set_last_file_edited(havename);
	if (bp2any_wp(bp) && bp2any_wp(havebp))
	    zotwp(bp);
    }
#if OPT_TAGS
    else if (tname) {
	cmdlinetag(tname);
	didtag = TRUE;
    }
#endif
    msg = s_NULL;
    if (helpflag) {
	if (vl_help(TRUE, 1) != TRUE) {
	    msg =
		"[Problem with help information. Type \":quit\" to exit if you wish]";
	}
    } else {
	msg = "[Use ^A-h, ^X-h, or :help to get help]";
    }

    /* honor command-line actions */
    if (gotoflag + (search_exp != 0)
#if OPT_TAGS
	+ (tname ? 1 : 0)
#endif
	> 1) {
#if OPT_TAGS
	msg = "[Search, goto and tag are used one at a time]";
#else
	msg = "[Cannot search and goto at the same time]";
#endif
    } else if (gotoflag) {
	if (gotoline(gline != 0, gline) == FALSE) {
	    msg = "[Not that many lines in buffer]";
	    (void) gotoeob(FALSE, 1);
	}
    } else if (search_exp) {
	FreeIfNeeded(gregexp);
	searchpat = tb_string(search_exp->pat);
	gregexp = search_exp->reg;
	(void) forwhunt(FALSE, 0);
#if OPT_TAGS
    } else if (tname && !didtag) {
	cmdlinetag(tname);
#endif
    }
#if OPT_POPUP_MSGS
    purge_msgs();
#endif
    if (startstat == TRUE)	/* else there's probably an error message */
	mlforce(msg);

  begin:
#if SYS_VMS
    if (init_descrip) {
	/*
	 * All terminal inits are complete.  Switch to new screen
	 * resolution specified from command line.
	 */
	(void) term.setdescrip(init_descrip);
    }
#endif
    (void) update(FALSE);

#if DISP_NTWIN
    /* Draw winvile's main window, for the very first time. */
    winvile_start();
#endif

#if OPT_POPUP_MSGS
    if (global_g_val(GMDPOPUP_MSGS) && (startstat != TRUE)) {
	bp = bfind(MESSAGES_BufName, BFSCRTCH);
	bsizes(bp);
	TRACE(("Checking size of popup messages: %d\n", bp->b_linecount));
	if (bp->b_linecount > 1) {
	    int save = global_g_val(GMDPOPUP_MSGS);
	    set_global_g_val(GMDPOPUP_MSGS, TRUE);
	    popup_msgs();
	    tb_init(&mlsave, EOS);
	    set_global_g_val(GMDPOPUP_MSGS, save);
	}
    }
    if (global_g_val(GMDPOPUP_MSGS) == -TRUE)
	set_global_g_val(GMDPOPUP_MSGS, FALSE);
#endif

#ifdef GMDCD_ON_OPEN
    /* reset a one-shot, e.g., for winvile's drag/drop code */
    if (global_g_val(GMDCD_ON_OPEN) < 0) {
	set_directory_from_file(curbp);
	set_global_g_val(GMDCD_ON_OPEN, FALSE);
    }
#endif

    /* We won't always be able to show messages before the screen is
     * initialized.  Give it one last chance.
     */
    if ((startstat != TRUE) && tb_length(mlsave))
	mlforce("%*S", tb_length(mlsave), tb_values(mlsave));

    /* process commands */
    main_loop();

    /* NOTREACHED */
    return BADEXIT;
}

/* this is nothing but the main command loop */
static void
main_loop(void)
{
    const CMDFUNC *cfp = NULL;
    const CMDFUNC *last_cfp = NULL;
    const CMDFUNC *last_failed_motion_cfp = NULL;
    int s, c, f, n;

    for_ever {

#if OPT_WORKING && DOALLOC
	assert(allow_working_msg());
#endif
	hst_reset();

	/* vi doesn't let the cursor rest on the newline itself.  This
	   takes care of that. */
	/* if we're inserting, or will be inserting again, then
	   suppress.  this happens if we're using arrow keys
	   during insert */
	if (is_at_end_of_line(DOT) && (DOT.o > w_left_margin(curwp)) &&
	    !insertmode && !cmd_mouse_motion(cfp))
	    backchar(TRUE, 1);

	/* same goes for end-of-file -- I'm actually not sure if
	   this can ever happen, but I _am_ sure that it's
	   a lot safer not to let it... */
	if (is_header_line(DOT, curbp) && !is_empty_buf(curbp))
	    (void) backline(TRUE, 1);

	/* start recording for '.' command */
	dotcmdbegin();

	/* bring the screen up to date */
	s = update(FALSE);

	/* get a user command */
	kbd_mac_check();
	c = kbd_seq();

	/* reset the contents of the command/status line */
	if (kbd_length() > 0) {
	    mlerase();
	    if (s != SORTOFTRUE)	/* did nothing due to typeahead */
		(void) update(FALSE);
	}

	f = FALSE;
	n = 1;

#if VILE_SOMEDAY
/* insertion is too complicated to pop in
	and out of so glibly...   -pgf */
#ifdef insertmode
	/* FIXME: Paul and Tom should check this over. */
	if (insertmode != FALSE) {
	    if (!kbd_replaying(FALSE))
		mayneedundo();
	    unkeystroke(c);
	    insert(f, n);
	    dotcmdfinish();
	    continue;
	}
#endif /* insertmode */
#endif /* SOMEDAY */

	do_repeats(&c, &f, &n);

	kregflag = 0;

	/* flag the first time through for some commands -- e.g. subst
	   must know to not prompt for strings again, and pregion
	   must only restart the p-lines buffer once for each
	   command. */
	calledbefore = FALSE;

	/* convert key code to a function pointer */
	cfp = DefaultKeyBinding(c);

	if (cfp == &f_dotcmdplay &&
	    (last_cfp == &f_undo ||
	     last_cfp == &f_forwredo ||
	     last_cfp == &f_backundo ||
	     last_cfp == &f_inf_undo))
	    cfp = &f_inf_undo;

	s = execute(cfp, f, n);

	last_cfp = cfp;

	/* stop recording for '.' command */
	dotcmdfinish();

	/* if we had an error in playback, stop right away, since the remaining
	 * characters may be from the middle of a change-command which failed
	 * due to invalid motion.
	 */
	if ((s == FALSE) && (dotcmdactive == PLAY)) {
	    dotcmdactive = 0;
	}

	/* If this was a motion that failed, sound the alarm (like vi),
	 * but limit it to once, in case the user is holding down the
	 * autorepeat-key.
	 */
	if ((cfp != NULL)
	    && ((cfp->c_flags & MOTION) != 0)
	    && (s == FALSE)) {
	    if (cfp != last_failed_motion_cfp ||
		global_g_val(GMDMULTIBEEP)) {
		last_failed_motion_cfp = cfp;
		kbd_alarm();
	    }
	} else {
	    last_failed_motion_cfp = NULL;	/* avoid noise! */
	}

	attrib_matches();
	regionshape = EXACT;

	/*
	 * Some perl scripts may not reset this flag.
	 */
	vile_is_busy = FALSE;

    }
}

/* attempt to locate the executable that contains our code.
* leave its directory name in exec_pathname and shorten prog_arg
* to the simple filename (no path).
*/
static void
get_executable_dir(void)
{
#if SYS_UNIX || SYS_VMS
    char temp[NFILEN];
    char *s, *t;

    if (last_slash(prog_arg) == NULL) {
	/* If there are no slashes, we can guess where we came from,
	 */
	if ((s = cfg_locate(prog_arg, FL_PATH | FL_EXECABLE)) != 0)
	    s = strmalloc(s);
    } else {
	/* if there _are_ slashes, then argv[0] was either
	 * absolute or relative. lengthen_path figures it out.
	 */
	s = strmalloc(lengthen_path(strcpy(temp, prog_arg)));
    }
    if (s == 0)
	return;

    t = pathleaf(s);
    if (t != s) {
	/*
	 * On a unix host, 't' points past slash.  On a VMS host,
	 * 't' points to first char after the last ':' or ']' in
	 * the exe's path.
	 */
	prog_arg = strmalloc(t);
	*t = EOS;
	exec_pathname = strmalloc(lengthen_path(strcpy(temp, s)));
    }
    free(s);
#endif
}

void
tidy_exit(int code)
{
#if OPT_PERL
    perl_exit();
#endif
#if DISP_NTWIN
    winvile_cleanup();
#endif
#if DISP_X11
    term.close();		/* need this if $xshell left subprocesses */
#endif
    ttclean(TRUE);
#if SYS_UNIX
    setup_handler(SIGHUP, SIG_IGN);
#endif
    ExitProgram(code);
}

#ifndef strmalloc
char *
strmalloc(const char *s)
{
    char *ns;
    beginDisplay();
    ns = castalloc(char, strlen(s) + 1);
    if (ns != 0)
	(void) strcpy(ns, s);
    endofDisplay();
    return ns;
}
#endif

int
no_memory(const char *s)
{
    mlforce("[%s] %s", out_of_mem, s);
    return FALSE;
}

#define setIntValue(vp, value) vp->v.i = value
#define setPatValue(vp, value) vp->v.r = new_regexval(value, TRUE)
#define setTxtValue(vp, value) vp->v.p = strmalloc(value)

#define setINT(name,value)  case name: setIntValue(d,value); break
#define setPAT(name,value)  case name: setPatValue(d,value); break;
#define setTXT(name,value)  case name: setTxtValue(d,value); break

#define DFT_FENCE_BEGIN "/\\*"
#define DFT_FENCE_END   "\\*/"

#define DFT_FENCE_IF    "^\\s*#\\s*if"
#define DFT_FENCE_ELIF  "^\\s*#\\s*elif\\>"
#define DFT_FENCE_ELSE  "^\\s*#\\s*else\\>"
#define DFT_FENCE_FI    "^\\s*#\\s*endif\\>"

			/* where do paragraphs start? */
#define DFT_PARAGRAPHS  "^\\.[ILPQ]P\\>\\|^\\.P\\>\\|\
^\\.LI\\>\\|^\\.[plinb]p\\>\\|^\\.\\?\\s*$"

			/* where do comments start and end */
#define DFT_COMMENTS    "^\\s*/\\?\\(\\s*[#*>/]\\)\\+/\\?\\s*$"

#define DFT_CMT_PREFIX  "^\\s*\\(\\(\\s*[#*>]\\)\\|\\(///*\\)\\)\\+"

			/* where do sections start? */
#define DFT_SECTIONS    "^[{\014]\\|^\\.[NS]H\\>\\|^\\.HU\\?\\>\\|\
^\\.[us]h\\>\\|^+c\\>"

			/* where do sentences start? */
#define DFT_SENTENCES   "[.!?][])\"']* \\?$\\|[.!?][])\"']*  \\|\
^\\.[ILPQ]P\\>\\|^\\.P\\>\\|^\\.LI\\>\\|^\\.[plinb]p\\>\\|^\\.\\?\\s*$"

#if SYS_MSDOS			/* actually, 16-bit ints */
#define DFT_TIMEOUTUSERVAL 30000
#else
#define DFT_TIMEOUTUSERVAL 60000
#endif

#if OPT_MSDOS_PATH
#define DFT_BACKUPSTYLE ".bak"
#else
#define DFT_BACKUPSTYLE "off"
#endif

#if SYS_VMS
#define DFT_CSUFFIX "\\.\\(\\([CHIS]\\)\\|CC\\|CXX\\|HXX\\)\\(;[0-9]*\\)\\?$"
#endif
#if SYS_MSDOS
#define DFT_CSUFFIX "\\.\\(\\([chis]\\)\\|cc\\|cpp\\|cxx\\|hxx\\)$"
#endif
#ifndef DFT_CSUFFIX		/* UNIX or OS2/HPFS (mixed-case names) */
#define DFT_CSUFFIX "\\.\\(\\([CchisS]\\)\\|CC\\|cc\\|cpp\\|cxx\\|hxx\\|scm\\)$"
#endif

#define DFT_FENCE_CHARS "{}()[]"
#define DFT_CINDENT_CHARS ":#" DFT_FENCE_CHARS

/*
 * For the given class/mode, initialize the VAL struct to the default value
 * for that mode.
 */
void
init_mode_value(struct VAL *d, MODECLASS v_class, int v_which)
{
    static const char expand_chars[] =
    {EXPC_THIS, EXPC_THAT, EXPC_SHELL, EXPC_TOKEN, EXPC_RPAT, 0};

    switch (v_class) {
    case UNI_MODE:
	switch (v_which) {
	    setINT(GMDABUFF, TRUE);	/* auto-buffer */
	    setINT(GMDALTTABPOS, FALSE);	/* emacs-style tab positioning */
	    setINT(GMDERRORBELLS, TRUE);	/* alarms are noticeable */
	    setINT(GMDEXPAND_PATH, FALSE);
	    setINT(GMDIMPLYBUFF, FALSE);	/* imply-buffer */
	    setINT(GMDMULTIBEEP, TRUE);		/* multiple beeps for multiple motion failures */
	    setINT(GMDREMAP, TRUE);	/* allow remapping by default */
	    setINT(GMDRONLYRONLY, FALSE);	/* Set readonly-on-readonly */
	    setINT(GMDRONLYVIEW, FALSE);	/* Set view-on-readonly */
	    setINT(GMDSMOOTH_SCROLL, FALSE);
	    setINT(GMDSPACESENT, TRUE);		/* add two spaces after each sentence */
	    setINT(GMDWARNRENAME, TRUE);	/* warn before renaming a buffer */
	    setINT(GMDWARNREREAD, TRUE);	/* warn before rereading a buffer */
	    setINT(GMDWARNUNREAD, TRUE);	/* warn if quitting without looking at all buffers */
	    setINT(GVAL_MAPLENGTH, 1200);
	    setINT(GVAL_MINI_HILITE, VAREV);	/* reverse hilite */
	    setINT(GVAL_PRINT_HIGH, 0);
	    setINT(GVAL_PRINT_LOW, 0);
	    setINT(GVAL_REPORT, 5);	/* report changes */
	    setINT(GVAL_TIMEOUTUSERVAL, 30000);		/* how long to wait for user seq */
	    setINT(GVAL_TIMEOUTVAL, 500);	/* how long to wait for ESC seq */
	    setTXT(GVAL_EXPAND_CHARS, expand_chars);
#ifdef GMDALL_VERSIONS
	    setINT(GMDALL_VERSIONS, FALSE);
#endif
#ifdef GMDCD_ON_OPEN
	    setINT(GMDCD_ON_OPEN, cd_on_open);
#endif
#ifdef GMDDIRC
	    setINT(GMDDIRC, FALSE);	/* directory-completion */
#endif
#ifdef GMDFLASH
	    setINT(GMDFLASH, FALSE);	/* beeps beep by default */
#endif
#ifdef FORCE_CONSOLE_RESURRECTED
# ifdef GMDFORCE_CONSOLE	/* deprecated (unused) */
	    setINT(GMDFORCE_CONSOLE, is_win95());
# endif
#endif
#ifdef GMDGLOB
	    setINT(GMDGLOB, TRUE);
#endif
#ifdef GMDHEAPSIZE
	    setINT(GMDHEAPSIZE, TRUE);	/* show heap usage */
#endif
#ifdef GMDHISTORY
	    setINT(GMDHISTORY, TRUE);
#endif
#ifdef GMDPOPUP_CHOICES
	    setINT(GMDPOPUP_CHOICES, TRUE);
#endif
#ifdef GMDPOPUPMENU
	    setINT(GMDPOPUPMENU, TRUE);		/* enable popup menu */
#endif
#ifdef GMDPOPUP_MSGS
	    setINT(GMDPOPUP_MSGS, -TRUE);	/* popup-msgs */
#endif
#ifdef GMDRESOLVE_LINKS
	    setINT(GMDRESOLVE_LINKS, FALSE);	/* set noresolve-links by default in case we've got NFS problems */
#endif
#ifdef GMDSWAPTITLE
	    setINT(GMDSWAPTITLE, FALSE);
#endif
#ifdef GMDUSEFILELOCK
	    setINT(GMDUSEFILELOCK, FALSE);	/* Use filelocks */
#endif
#ifdef GMDW32PIPES
	    setINT(GMDW32PIPES, is_winnt());	/* use native pipes?  */
#endif
#ifdef GMDWARNBLANKS
	    setINT(GMDWARNBLANKS, FALSE);	/* if filenames have blanks */
#endif
#ifdef GMDWORKING
	    setINT(GMDWORKING, TRUE);	/* we put up "working..." */
#endif
#ifdef GMDXTERM_MOUSE
	    setINT(GMDXTERM_MOUSE, FALSE);	/* mouse-clicking */
#endif
#ifdef GMDXTERM_TITLE
	    setINT(GMDXTERM_TITLE, FALSE);	/* xterm window-title */
#endif
#ifdef GVAL_BACKUPSTYLE
	    setTXT(GVAL_BACKUPSTYLE, DFT_BACKUPSTYLE);
#endif
#ifdef GVAL_BCOLOR
	    setINT(GVAL_BCOLOR, C_BLACK);	/* background color */
#endif
#ifdef GVAL_CCOLOR
	    setINT(GVAL_CCOLOR, ENUM_UNKNOWN);	/* cursor color */
#endif
#ifdef GVAL_CSUFFIXES
	    setPAT(GVAL_CSUFFIXES, DFT_CSUFFIX);
#endif
#ifdef GVAL_FCOLOR
	    setINT(GVAL_FCOLOR, C_WHITE);	/* foreground color */
#endif
#ifdef GVAL_FINDCFG
	    setTXT(GVAL_FINDCFG, "");
#endif
#ifdef GVAL_GLOB
	    setTXT(GVAL_GLOB, "!echo %s");
#endif
#ifdef GVAL_ICURSOR
	    setTXT(GVAL_ICURSOR, "0");	/* no insertion cursor */
#endif
#ifdef GVAL_MCOLOR
	    setINT(GVAL_MCOLOR, VAREV);		/* show in reverse */
#endif
#ifdef GVAL_POPUP_CHOICES
	    setINT(GVAL_POPUP_CHOICES, POPUP_CHOICES_DELAYED);
#endif
#ifdef GVAL_REDIRECT_KEYS
	    setTXT(GVAL_REDIRECT_KEYS,
		   "F5::S,F10::S,F11::S,F7::F,F5:C:,F9::Y");
#endif
#ifdef GVAL_SCROLLPAUSE
	    setINT(GVAL_SCROLLPAUSE, 0);
#endif
#ifdef GVAL_VTFLASH
	    setINT(GVAL_VTFLASH, VTFLASH_OFF);	/* hardwired flash off */
#endif
	default:
	    setIntValue(d, 0);
	    break;
	}
	break;
    case BUF_MODE:
	switch (v_which) {
	    setINT(MDAIND, FALSE);	/* auto-indent */
	    setINT(MDASAVE, FALSE);	/* auto-save */
	    setINT(MDBACKLIMIT, TRUE);	/* limit backspacing to insert point */
	    setINT(MDCINDENT, FALSE);	/* C-style indent */
	    setINT(MDDOS, CRLF_LINES);	/* on by default on DOS, off others */
	    setINT(MDIGNCASE, FALSE);	/* exact matches */
	    setINT(MDMAGIC, TRUE);	/* magic searches */
	    setINT(MDMETAINSBIND, TRUE);	/* honor meta-bindings when in insert mode */
	    setINT(MDNEWLINE, TRUE);	/* trailing-newline */
	    setINT(MDREADONLY, FALSE);	/* readonly */
	    setINT(MDSHOWMAT, FALSE);	/* show-match */
	    setINT(MDSHOWMODE, TRUE);	/* show-mode */
	    setINT(MDSWRAP, TRUE);	/* scan wrap */
	    setINT(MDTABINSERT, TRUE);	/* allow tab insertion */
	    setINT(MDTAGSRELTIV, FALSE);	/* path relative tag lookups */
	    setINT(MDTERSE, FALSE);	/* terse messaging */
	    setINT(MDUNDOABLE, TRUE);	/* undo stack active */
	    setINT(MDUNDO_DOS_TRIM, FALSE);	/* undo dos trimming */
	    setINT(MDVIEW, FALSE);	/* view-only */
	    setINT(MDWRAP, FALSE);	/* wrap */
	    setINT(MDYANKMOTION, TRUE);		/* yank-motion */
	    setINT(VAL_ASAVECNT, 256);	/* autosave count */
	    setINT(VAL_PERCENT_CRLF, 50);	/* threshold for crlf conversion */
	    setINT(VAL_RECORD_SEP, RS_DEFAULT);
	    setINT(VAL_SHOW_FORMAT, SF_FOREIGN);
	    setINT(VAL_SWIDTH, 8);	/* shiftwidth */
	    setINT(VAL_TAB, 8);	/* tab stop */
	    setINT(VAL_TAGLEN, 0);	/* significant tag length */
	    setINT(VAL_UNDOLIM, 10);	/* undo limit */
	    setPAT(VAL_CMT_PREFIX, DFT_CMT_PREFIX);
	    setPAT(VAL_COMMENTS, DFT_COMMENTS);
	    setPAT(VAL_FENCE_BEGIN, DFT_FENCE_BEGIN);
	    setPAT(VAL_FENCE_ELIF, DFT_FENCE_ELIF);
	    setPAT(VAL_FENCE_ELSE, DFT_FENCE_ELSE);
	    setPAT(VAL_FENCE_END, DFT_FENCE_END);
	    setPAT(VAL_FENCE_FI, DFT_FENCE_FI);
	    setPAT(VAL_FENCE_IF, DFT_FENCE_IF);
	    setPAT(VAL_PARAGRAPHS, DFT_PARAGRAPHS);
	    setPAT(VAL_SECTIONS, DFT_SECTIONS);
	    setPAT(VAL_SENTENCES, DFT_SENTENCES);
	    setTXT(VAL_CINDENT_CHARS, DFT_CINDENT_CHARS);	/* C-style indent flags */
	    setTXT(VAL_FENCES, "{}()[]");	/* fences */
	    setTXT(VAL_TAGS, "tags");	/* tags filename */
#ifdef MDCHK_MODTIME
	    setINT(MDCHK_MODTIME, FALSE);	/* modtime-check */
#endif
#ifdef MDCMOD
	    setINT(MDCMOD, FALSE);	/* C mode */
#endif
#ifdef MDCRYPT
	    setINT(MDCRYPT, FALSE);	/* crypt */
#endif
#ifdef MDHILITE
	    setINT(MDHILITE, TRUE);	/* syntax-highlighting */
#endif
#ifdef MDHILITEOVERLAP
	    setINT(MDHILITEOVERLAP, TRUE);	/* overlap visual-matches */
#endif
#ifdef MDLOCKED
	    setINT(MDLOCKED, FALSE);	/* LOCKED */
#endif
#ifdef MDUPBUFF
	    setINT(MDUPBUFF, TRUE);	/* animated */
#endif
#ifdef VAL_AUTOCOLOR
	    setINT(VAL_AUTOCOLOR, 0);	/* auto syntax coloring timeout */
#endif
#ifdef VAL_HILITEMATCH
	    setINT(VAL_HILITEMATCH, 0);		/* no hilite */
#endif
#ifdef VAL_FENCE_LIMIT
	    setINT(VAL_FENCE_LIMIT, 10);	/* fences iteration timeout */
#endif
#ifdef VAL_LOCKER
	    setTXT(VAL_LOCKER, "");	/* Name locker */
#endif
#ifdef VAL_RECORD_FORMAT
	    setINT(VAL_RECORD_FORMAT, FAB$C_UDF);
#endif
#ifdef VAL_RECORD_LENGTH
	    setINT(VAL_RECORD_LENGTH, 0);
#endif
#ifdef VAL_C_SWIDTH
	    setINT(VAL_C_SWIDTH, 8);	/* C file shiftwidth */
#endif
#ifdef VAL_C_TAB
	    setINT(VAL_C_TAB, 8);	/* C file tab stop */
#endif
	default:
	    setIntValue(d, 0);
	    break;
	}
	break;
    case WIN_MODE:
	switch (v_which) {
	    setINT(WMDHORSCROLL, TRUE);		/* horizontal scrolling */
	    setINT(WMDLIST, FALSE);	/* list-mode */
	    setINT(WMDNUMBER, FALSE);	/* number */
	    setINT(WVAL_SIDEWAYS, 0);	/* list-mode */
#ifdef WMDLINEWRAP
	    setINT(WMDLINEWRAP, FALSE);		/* line-wrap */
#endif
#ifdef WMDTERSELECT
	    setINT(WMDTERSELECT, TRUE);		/* terse selections */
#endif
	default:
	    setIntValue(d, 0);
	    break;
	}
	break;
    default:
	setIntValue(d, 0);
	break;
    }
}

	/*
	 * The $filename-expr expression is used with a trailing '*' to repeat
	 * the last field.  Otherwise it is complete.
	 *
	 * Limitations - the expressions are used in the error finder, which
	 * has to choose the first pattern which matches a filename embedded
	 * in other text.  UNIX-style pathnames could have embedded blanks,
	 * but it's not common.  It is common on win32, but is not often a
	 * problem with grep, etc., since the full pathname of the file is not
	 * usually given.
	 */
#if OPT_MSDOS_PATH
#define DFT_FILENAME_EXPR "\\([a-zA-Z]:\\)\\?[^ \t\":*?<>|]"
#else
#if OPT_VMS_PATH
#define DFT_FILENAME_EXPR "[-/\\w.;\\[\\]<>$:]"
#else /* UNIX-style */
#define DFT_FILENAME_EXPR "[-/\\w.~]"
#endif
#endif

#define DFT_HELP_FILE "vile.hlp"

#define DFT_MENU_FILE "vilemenu.rc"

#ifdef VILE_LIBDIR_PATH
#define DFT_LIBDIR_PATH VILE_LIBDIR_PATH
#else
#define DFT_LIBDIR_PATH ""
#endif

#if SYS_MSDOS || SYS_OS2 || SYS_WINNT || SYS_VMS
#define DFT_STARTUP_FILE "vile.rc"
#else /* SYS_UNIX */
#define DFT_STARTUP_FILE ".vilerc"
#endif

#define DFT_MLFORMAT \
"%-%i%- %b %m:: :%f:is : :%=%F: : :%l:(:,:%c::) :%p::% :%S%-%-%|"

#define DFT_POSFORMAT \
"Line %{$curline}d of %{$blines}d,\
 Col %{$curcol}d of %{$lcols}d,\
 Char %{$curchar}d of %{$bchars}d\
 (%P%%) char is 0x%{$char}x or 0%{$char}o"

static char *
default_help_file(void)
{
    char *result;
    if ((result = getenv("VILE_HELP_FILE")) == 0)
	result = DFT_HELP_FILE;
    return result;
}

static char *
default_libdir_path(void)
{
    char *result;
    if ((result = getenv("VILE_LIBDIR_PATH")) == 0)
	result = DFT_LIBDIR_PATH;
    return result;
}

#if OPT_MENUS
static char *
default_menu_file(void)
{
    static char default_menu[] = DFT_MENU_FILE;
    char temp[NSTRING];
    char *menurc = getenv("VILE_MENU");

    if (menurc == NULL || *menurc == EOS) {
	sprintf(temp, "%.*s_MENU", (int) (sizeof(temp) - 6), prognam);
	mkupper(temp);
	menurc = getenv(temp);
	if (menurc == NULL || *menurc == EOS) {
	    menurc = default_menu;
	}
    }

    return menurc;
}
#endif

static char *
default_startup_file(void)
{
    char *result;
    if ((result = getenv("VILE_STARTUP_FILE")) == 0)
	result = DFT_STARTUP_FILE;
    return result;
}

static char *
default_startup_path(void)
{
    char *result = 0;
    char *s;

    if ((s = getenv("VILE_STARTUP_PATH")) != 0) {
	append_to_path_list(&result, s);
    } else {
#if SYS_MSDOS || SYS_OS2 || SYS_WINNT
	append_to_path_list(&result, "/sys/public");
	append_to_path_list(&result, "/usr/bin");
	append_to_path_list(&result, "/bin");
	append_to_path_list(&result, "/");
#else
#if SYS_VMS
	append_to_path_list(&result, "sys$login");
	append_to_path_list(&result, "sys$sysdevice:[vmstools]");
	append_to_path_list(&result, "sys$library");
#else /* SYS_UNIX */
#if defined(VILE_STARTUP_PATH)
	append_to_path_list(&result, VILE_STARTUP_PATH);
#else
	append_to_path_list(&result, "/usr/local/lib");
	append_to_path_list(&result, "/usr/local");
	append_to_path_list(&result, "/usr/lib");
#endif /* VILE_STARTUP_PATH */
#endif /* SYS_VMS... */
#endif /* SYS_MSDOS... */
    }
#ifdef HELP_LOC
    append_to_path_list(&result, HELP_LOC);
#endif
    return result;
}

#if OPT_EVAL
/*
 * Returns a string representing the default/initial value of the given
 * state-variable.
 */
char *
init_state_value(int which)
{
    char *result = 0;
    int allocated = FALSE;

    switch (which) {
#if OPT_FINDERR
    case VAR_FILENAME_EXPR:
	result = DFT_FILENAME_EXPR;
	break;
#endif
    case VAR_HELPFILE:
	result = default_help_file();
	break;
    case VAR_LIBDIR_PATH:
	result = default_libdir_path();
	break;
#if OPT_MENUS
    case VAR_MENU_FILE:
	result = default_menu_file();
	break;
#endif
#if OPT_MLFORMAT
    case VAR_MLFORMAT:
	result = DFT_MLFORMAT;
	break;
#endif
#if OPT_POSFORMAT
    case VAR_POSFORMAT:
	result = DFT_POSFORMAT;
	break;
#endif
    case VAR_PROMPT:
	result = ": ";
	break;
    case VAR_REPLACE:
	result = "";
	break;
    case VAR_STARTUP_FILE:
	result = default_startup_file();
	break;
    case VAR_STARTUP_PATH:
	result = default_startup_path();
	allocated = TRUE;
	break;
    }
    return (result != 0 && !allocated) ? strmalloc(result) : result;
}
#endif /* OPT_EVAL */

#if OPT_REBIND
/*
 * Cache the length of kbindtbl[].
 */
static size_t
len_kbindtbl(void)
{
    static size_t result = 0;
    if (result == 0)
	while (kbindtbl[result].k_cmd != 0)
	    result++;
    return result + 1;
}

/*
 * At startup, construct copies of kbindtbl[] for the alternate bindings, so
 * we can have separate bindings of special keys from the default bindings.
 */
static void
copy_kbindtbl(BINDINGS * dst)
{
    KBIND *result;
    size_t len = len_kbindtbl();

    if (dst->kb_special == kbindtbl) {
	if ((result = typecallocn(KBIND, len)) == 0) {
	    (void) no_memory("copy_kbindtbl");
	    return;
	}
	dst->kb_special = dst->kb_extra = result;
    } else {
	result = dst->kb_special;
    }
    while (len-- != 0) {
	result[len] = kbindtbl[len];
    }
}
#endif

#if OPT_FILTER && defined(WIN32)
extern void flt_array(void);
#endif

static void
global_val_init(void)
{
    int i;

    TRACE((T_CALLED "global_val_init()\n"));
#if OPT_FILTER && defined(WIN32)
    flt_array();
#endif
    /* set up so the global value pointers point at the global
       values.  we never actually use the global pointers
       directly, but when buffers get a copy of the
       global_b_values structure, the pointers will still point
       back at the global values, which is what we want */
    for (i = 0; i <= NUM_G_VALUES; i++) {
	make_local_val(global_g_values.gv, i);
	init_mode_value(&global_g_values.gv[i], UNI_MODE, i);
    }

    for (i = 0; i <= NUM_B_VALUES; i++) {
	make_local_val(global_b_values.bv, i);
	init_mode_value(&global_b_values.bv[i], BUF_MODE, i);
    }

    for (i = 0; i <= NUM_W_VALUES; i++) {
	make_local_val(global_w_values.wv, i);
	init_mode_value(&global_w_values.wv[i], WIN_MODE, i);
    }

#if OPT_MAJORMODE
    /*
     * Built-in majormodes (see 'modetbl')
     */
    alloc_mode("c", TRUE);
    alloc_mode("vile", TRUE);

    set_submode_val("c", VAL_SWIDTH, 8);	/* C file shiftwidth */
    set_submode_val("c", VAL_TAB, 8);	/* C file tab stop */
    set_submode_val("c", MDCINDENT, TRUE);	/* C-style indent */
    set_submode_txt("c", VAL_CINDENT_CHARS, DFT_CINDENT_CHARS);		/* C-style indent flags */

    set_majormode_rexp("c", MVAL_SUFFIXES, DFT_CSUFFIX);
#endif

#if OPT_EVAL
    /*
     * Note that not all state variables can be initialized explicitly, many
     * are either read-only, or have too many side-effects to set blindly.
     */
    for (i = 0; i < Num_StateVars; i++) {
	char *s;
	if ((s = init_state_value(i)) != 0) {
	    set_state_variable(statevars[i], s);
	    free(s);
	}
    }
#else
    helpfile = default_help_file();
    startup_file = default_startup_file();
    startup_path = default_startup_path();
    libdir_path = default_libdir_path();
#if OPT_MENUS
    menu_file = default_menu_file();
#endif
#if OPT_MLFORMAT
    modeline_format = strmalloc(DFT_MLFORMAT);
#endif
#if OPT_POSFORMAT
    position_format = strmalloc(DFT_POSFORMAT);
#endif
#if OPT_FINDERR
    filename_expr = strmalloc(DFT_FILENAME_EXPR);
#endif
#endif

    /*
     * Initialize the "normal" bindings for insert and command mode to
     * the motion-commands that are safe if insert-exec mode is enabled.
     */
#if OPT_REBIND
    copy_kbindtbl(&sel_bindings);
    copy_kbindtbl(&ins_bindings);
    copy_kbindtbl(&cmd_bindings);
#endif
    for (i = 0; i < N_chars; i++) {
	const CMDFUNC *cfp;

	sel_bindings.kb_normal[i] = dft_bindings.kb_normal[i];
	ins_bindings.kb_normal[i] = 0;
	cmd_bindings.kb_normal[i] = dft_bindings.kb_normal[i];
	if (i < 32
	    && (cfp = dft_bindings.kb_normal[i]) != 0
	    && (cfp->c_flags & (GOAL | MOTION)) != 0) {
	    ins_bindings.kb_normal[i] =
		cmd_bindings.kb_normal[i] = cfp;
	}
    }
    returnVoid();
}

#if SYS_UNIX || SYS_MSDOS || SYS_OS2 || SYS_WINNT || SYS_VMS

/* ARGSUSED */
SIGT
catchintr(int ACTUAL_SIG_ARGS)
{
    am_interrupted = TRUE;
#if SYS_MSDOS || SYS_OS2 || SYS_WINNT
    sgarbf = TRUE;		/* there's probably a ^C on the screen. */
#endif
    setup_handler(SIGINT, catchintr);
    if (im_waiting(-1))
	longjmp(read_jmp_buf, signo);
    SIGRET;
}
#endif

#ifndef interrupted		/* i.e. unless it's a macro */
int
interrupted(void)
{
#if SYS_MSDOS && CC_DJGPP

    if (_go32_was_ctrl_break_hit() != 0) {
	while (keystroke_avail())
	    (void) keystroke();
	return TRUE;
    }
    if (was_ctrl_c_hit() != 0) {
	while (keystroke_avail())
	    (void) keystroke();
	return TRUE;
    }

    if (am_interrupted)
	return TRUE;
    return FALSE;
#endif
}
#endif

void
not_interrupted(void)
{
    am_interrupted = FALSE;
#if SYS_MSDOS
# if CC_DJGPP
    (void) _go32_was_ctrl_break_hit();	/* flush any pending kbd ctrl-breaks */
    (void) was_ctrl_c_hit();	/* flush any pending kbd ctrl-breaks */
# endif
#endif
}

#if SYS_MSDOS
# if CC_WATCOM
int
wat_crit_handler(unsigned deverror, unsigned errcode, unsigned *devhdr)
{
    _hardresume((int) _HARDERR_FAIL);
    return (int) _HARDERR_FAIL;
}

#else
void
dos_crit_handler(void)
{
#  if ! CC_DJGPP
    _hardresume(_HARDERR_FAIL);
#  endif
}

#  endif
#endif

static void
siginit(void)
{
#if SYS_UNIX
    setup_handler(SIGINT, catchintr);
    setup_handler(SIGHUP, imdying);
#ifdef SIGBUS
    setup_handler(SIGBUS, imdying);
#endif
#ifdef SIGSYS
    setup_handler(SIGSYS, imdying);
#endif
    setup_handler(SIGSEGV, imdying);
    setup_handler(SIGTERM, imdying);
/* #define VILE_DEBUG 1 */
#ifdef VILE_DEBUG
    setup_handler(SIGQUIT, imdying);
#else
    setup_handler(SIGQUIT, SIG_IGN);
#endif
    setup_handler(SIGPIPE, SIG_IGN);
#if defined(SIGWINCH) && ! DISP_X11
    setup_handler(SIGWINCH, sizesignal);
#endif
#else
# if SYS_MSDOS
    setup_handler(SIGINT, catchintr);
#  if CC_DJGPP
    _go32_want_ctrl_break(TRUE);
    setcbrk(FALSE);
    want_ctrl_c(TRUE);
    hard_error_catch_setup();
#  else
#   if CC_WATCOM
    {
	/* clean up Warning from Watcom C */
	void *ptrfunc = wat_crit_handler;
	_harderr(ptrfunc);
    }
#   else			/* CC_TURBO */
    _harderr(dos_crit_handler);
#   endif
#  endif
# endif
# if SYS_OS2 || SYS_WINNT
    setup_handler(SIGINT, catchintr);
# endif
#endif
}

static void
siguninit(void)
{
#if SYS_MSDOS
# if CC_DJGPP
    _go32_want_ctrl_break(FALSE);
    want_ctrl_c(FALSE);
    hard_error_teardown();
    setcbrk(TRUE);
# endif
#endif
}

/* do number processing if needed */
static void
do_num_proc(int *cp, int *fp, int *np)
{
    register int c, f, n;
    register int mflag;
    register int oldn;

    c = *cp;

    if (isCntrl(c) || isSpecial(c))
	return;

    f = *fp;
    n = *np;
    if (f)
	oldn = n;
    else
	oldn = 1;
    n = 1;

    if (isDigit(c) && c != '0') {
	n = 0;			/* start with a zero default */
	f = TRUE;		/* there is a # arg */
	mflag = 1;		/* current minus flag */
	while ((isDigit(c) && !isSpecial(c)) || (c == '-')) {
	    if (c == '-') {
		/* already hit a minus or digit? */
		if ((mflag == -1) || (n != 0))
		    break;
		mflag = -1;
	    } else {
		n = n * 10 + (c - '0');
	    }
	    if ((n == 0) && (mflag == -1))	/* lonely - */
		mlwrite("arg:");
	    else
		mlwrite("arg: %d", n * mflag);

	    c = kbd_seq();	/* get the next key */
	}
	n = n * mflag;		/* figure in the sign */
    }
    *cp = c;
    *fp = f;
    *np = n * oldn;
}

/* do emacs ^U-style repeat argument processing -- vile binds this to 'K' */
static void
do_rept_arg_proc(int *cp, int *fp, int *np)
{
    register int c, f, n;
    register int mflag;
    register int oldn;
    c = *cp;

    if (c != reptc)
	return;

    f = *fp;
    n = *np;

    if (f)
	oldn = n;
    else
	oldn = 1;

    n = 4;			/* start with a 4 */
    f = TRUE;			/* there is a # arg */
    mflag = 0;			/* that can be discarded. */
    mlwrite("arg: %d", n);
    while ((isDigit(c = kbd_seq()) && !isSpecial(c))
	   || c == reptc || c == '-') {
	if (c == reptc)
	    n = n * 4;

	/*
	 * If dash, and start of argument string, set arg.
	 * to -1.  Otherwise, insert it.
	 */
	else if (c == '-') {
	    if (mflag)
		break;
	    n = 0;
	    mflag = -1;
	}
	/*
	 * If first digit entered, replace previous argument
	 * with digit and set sign.  Otherwise, append to arg.
	 */
	else {
	    if (!mflag) {
		n = 0;
		mflag = 1;
	    }
	    n = 10 * n + c - '0';
	}
	mlwrite("arg: %d", (mflag >= 0) ? n : (n ? -n : -1));
    }
    /*
     * Make arguments preceded by a minus sign negative and change
     * the special argument "^U -" to an effective "^U -1".
     */
    if (mflag == -1) {
	if (n == 0)
	    n++;
	n = -n;
    }

    *cp = c;
    *fp = f;
    *np = n * oldn;
}

/* handle all repeat counts */
void
do_repeats(int *cp, int *fp, int *np)
{
    do_num_proc(cp, fp, np);
    do_rept_arg_proc(cp, fp, np);
    if (dotcmdactive == PLAY) {
	if (dotcmdarg)		/* then repeats are done by dotcmdcnt */
	    *np = 1;
    } else {
	/* then we want to cancel any dotcmdcnt repeats */
	if (*fp)
	    dotcmdarg = FALSE;
    }
}

/* the vi ZZ command -- write all, then quit */
int
zzquit(int f, int n)
{
    int thiscmd;
    int cnt;
    BUFFER *bp;

    thiscmd = lastcmd;
    cnt = any_changed_buf(&bp);
    if (cnt) {
	if (cnt > 1) {
	    mlprompt("Will write %d buffers.  %s ", cnt,
		     clexec ? s_NULL : "Repeat command to continue.");
	} else {
	    mlprompt("Will write buffer \"%s\".  %s ",
		     bp->b_bname,
		     clexec ? s_NULL : "Repeat command to continue.");
	}
	if (!clexec && !isnamedcmd) {
	    if (thiscmd != kbd_seq())
		return FALSE;
	}

	if (writeall(f, n, FALSE, TRUE, FALSE, FALSE) != TRUE) {
	    return FALSE;
	}

    } else if (!clexec && !isnamedcmd) {
	/* consume the next char. anyway */
	if (thiscmd != kbd_seq())
	    return FALSE;
    }
    return quit(f, n);
}

/*
 * attempt to write all changed buffers, and quit
 */
int
quickexit(int f, int n)
{
    register int status;
    if ((status = writeall(f, n, FALSE, TRUE, FALSE, FALSE)) == TRUE)
	status = quithard(f, n);	/* conditionally quit */
    return status;
}

/* Force quit by giving argument */
/* ARGSUSED */
int
quithard(int f GCC_UNUSED, int n GCC_UNUSED)
{
    return quit(TRUE, 1);
}

/*
 * Quit command. If an argument, always quit. Otherwise confirm if a buffer
 * has been changed and not written out.
 */
/* ARGSUSED */
int
quit(int f, int n GCC_UNUSED)
{
    int cnt;
    BUFFER *bp;
    const char *sadj, *sverb;

    run_a_hook(&exithook);

    if (f == FALSE) {
	cnt = any_changed_buf(&bp);
	sadj = "modified";
	sverb = "Write";
	if (cnt == 0 && global_g_val(GMDWARNUNREAD)) {
	    cnt = any_unread_buf(&bp);
	    sadj = "unread";
	    sverb = "Look at";
	}
	if (cnt != 0) {
	    if (cnt == 1)
		mlforce(
			   "Buffer \"%s\" is %s.  %s it, or use :q!",
			   bp->b_bname, sadj, sverb);
	    else
		mlforce(
			   "There are %d %s buffers.  %s them, or use :q!",
			   cnt, sadj, sverb);
	    return FALSE;
	}
    }
#if OPT_LCKFILES
    /* Release all placed locks */
    if (global_g_val(GMDUSEFILELOCK)) {
	for_each_buffer(bp) {
	    if (bp->b_active) {
		if (!b_val(curbp, MDLOCKED) &&
		    !b_val(curbp, MDVIEW))
		    release_lock(bp->b_fname);
	    }
	}
    }
#endif
    siguninit();
#if OPT_WORKING
    setup_handler(SIGALRM, SIG_IGN);
#endif
#if NO_LEAKS
    {
	beginDisplay();		/* ...this may take a while... */

	/* free all of the global data structures */
	onel_leaks();
	path_leaks();
	kbs_leaks();
	bind_leaks();
	map_leaks();
	tags_leaks();
	wp_leaks();
	bp_leaks();
	vt_leaks();
#if !SMALLER
	ev_leaks();
#endif
	mode_leaks();
	vars_leaks();
	fileio_leaks();
#if DISP_X11
	x11_leaks();
#endif

	free_local_vals(g_valnames, global_g_values.gv, global_g_values.gv);
	free_local_vals(b_valnames, global_b_values.bv, global_b_values.bv);
	free_local_vals(w_valnames, global_w_values.wv, global_w_values.wv);

	FreeAndNull(libdir_path);
	FreeAndNull(startup_path);
	FreeAndNull(gregexp);
	tb_free(&tb_matched_pat);
#if OPT_MLFORMAT
	FreeAndNull(modeline_format);
#endif
#if OPT_POSFORMAT
	FreeAndNull(position_format);
#endif
	FreeAndNull(helpfile);
	FreeAndNull(startup_file);

#if SYS_UNIX
	if (strcmp(exec_pathname, "."))
	    FreeAndNull(exec_pathname);
#endif
#if OPT_FILTER
	flt_leaks();
	filters_leaks();
#endif
	/* do these last, e.g., for tb_matched_pat */
	itb_leaks();
	tb_leaks();

	term.close();
	/* whatever is left over must be a leak */
	show_alloc();
	trace_leaks();
    }
#endif
    tidy_exit(GOODEXIT);
    /* NOTREACHED */
    return FALSE;
}

/* ARGSUSED */
int
writequit(int f GCC_UNUSED, int n)
{
    int s;
    s = filesave(FALSE, n);
    if (s != TRUE)
	return s;
    return quit(FALSE, n);
}

/*
 * Abort.
 * Beep the beeper. Kill off any keyboard macro, etc., that is in progress.
 * Sometimes called as a routine, to do general aborting of stuff.
 */
/* ARGSUSED */
int
esc_func(int f GCC_UNUSED, int n GCC_UNUSED)
{
    dotcmdactive = 0;
    regionshape = EXACT;
    doingopcmd = FALSE;
    do_sweep(FALSE);
    sweephack = FALSE;
    opcmd = 0;
    mlwarn("[Aborted]");
    return ABORT;
}

/* tell the user that this command is illegal while we are in
   VIEW (read-only) mode				*/

int
rdonly(void)
{
    mlwarn("[No changes are allowed while in \"view\" mode]");
    return FALSE;
}

/* ARGSUSED */
int
unimpl(int f GCC_UNUSED, int n GCC_UNUSED)
{
    mlwarn("[Sorry, that vi command is unimplemented in vile ]");
    return FALSE;
}

int
opercopy(int f, int n)
{
    return unimpl(f, n);
}

int
opermove(int f, int n)
{
    return unimpl(f, n);
}

int
opertransf(int f, int n)
{
    return unimpl(f, n);
}

int
operglobals(int f, int n)
{
    return unimpl(f, n);
}

int
opervglobals(int f, int n)
{
    return unimpl(f, n);
}

int
vl_source(int f, int n)
{
    return unimpl(f, n);
}

int
visual(int f, int n)
{
    return unimpl(f, n);
}

int
ex(int f, int n)
{
    return unimpl(f, n);
}

/* ARGSUSED */
/* user function that only returns true */
int
nullproc(int f GCC_UNUSED, int n GCC_UNUSED)
{
    return TRUE;
}

/* dummy functions for binding to key sequence prefixes */
/* ARGSUSED */
int
cntl_a_func(int f GCC_UNUSED, int n GCC_UNUSED)
{
    return TRUE;
}

/* ARGSUSED */
int
cntl_x_func(int f GCC_UNUSED, int n GCC_UNUSED)
{
    return TRUE;
}

/* ARGSUSED */
int
poundc_func(int f GCC_UNUSED, int n GCC_UNUSED)
{
    return TRUE;
}

/* ARGSUSED */
int
reptc_func(int f GCC_UNUSED, int n GCC_UNUSED)
{
    return TRUE;
}

/*----------------------------------------------------------------------------*/

/* initialize our version of the "chartypes" stuff normally in ctypes.h */
/* also called later, if charset-affecting modes change, for instance */
void
charinit(void)
{
    register int c;

    TRACE((T_CALLED "charinit() lo=%d, hi=%d\n",
	   global_g_val(GVAL_PRINT_LOW),
	   global_g_val(GVAL_PRINT_HIGH)));

    /* If we're using the locale functions, set our flags based on its
     * tables.  Note that just because you have 'setlocale()' doesn't mean
     * that the tables are present or correct.  But this is a start.
     */
#if OPT_LOCALE
    for (c = 0; c < N_chars; c++) {
	vl_chartypes_[c] = 0;
	if (sys_iscntrl(c))
	    vl_chartypes_[c] |= vl_cntrl;
	if (sys_isdigit(c))
	    vl_chartypes_[c] |= vl_digit;
	if (sys_islower(c))
	    vl_chartypes_[c] |= vl_lower;
	if (sys_isprint(c))
	    vl_chartypes_[c] |= vl_print;
	if (sys_ispunct(c))
	    vl_chartypes_[c] |= vl_punct;
	if (sys_isspace(c))
	    vl_chartypes_[c] |= vl_space;
	if (sys_isupper(c))
	    vl_chartypes_[c] |= vl_upper;
#ifdef vl_xdigit
	if (sys_isxdigit(c))
	    vl_chartypes_[c] |= vl_xdigit;
#endif
    }
#else /* ! OPT_LOCALE */
    (void) memset((char *) vl_chartypes_, 0, sizeof(vl_chartypes_));

    /* control characters */
    for (c = 0; c < ' '; c++)
	vl_chartypes_[c] |= vl_cntrl;
    vl_chartypes_[127] |= vl_cntrl;

    /* lowercase */
    for (c = 'a'; c <= 'z'; c++)
	vl_chartypes_[c] |= vl_lower;
#if OPT_ISO_8859
    for (c = 0xc0; c <= 0xd6; c++)
	vl_chartypes_[c] |= vl_lower;
    for (c = 0xd8; c <= 0xde; c++)
	vl_chartypes_[c] |= vl_lower;
#endif
    /* uppercase */
    for (c = 'A'; c <= 'Z'; c++)
	vl_chartypes_[c] |= vl_upper;
#if OPT_ISO_8859
    for (c = 0xdf; c <= 0xf6; c++)
	vl_chartypes_[c] |= vl_upper;
    for (c = 0xf8; c <= 0xff; c++)
	vl_chartypes_[c] |= vl_upper;
#endif

    /* digits */
    for (c = '0'; c <= '9'; c++)
	vl_chartypes_[c] |= vl_digit;
#ifdef vl_xdigit
    /* hex digits */
    for (c = '0'; c <= '9'; c++)
	vl_chartypes_[c] |= vl_xdigit;
    for (c = 'a'; c <= 'f'; c++)
	vl_chartypes_[c] |= vl_xdigit;
    for (c = 'A'; c <= 'F'; c++)
	vl_chartypes_[c] |= vl_xdigit;
#endif

    /* punctuation */
    for (c = '!'; c <= '/'; c++)
	vl_chartypes_[c] |= vl_punct;
    for (c = ':'; c <= '@'; c++)
	vl_chartypes_[c] |= vl_punct;
    for (c = '['; c <= '`'; c++)
	vl_chartypes_[c] |= vl_punct;
    for (c = L_CURLY; c <= '~'; c++)
	vl_chartypes_[c] |= vl_punct;
#if OPT_ISO_8859
    for (c = 0xa1; c <= 0xbf; c++)
	vl_chartypes_[c] |= vl_punct;
#endif

    /* printable */
    for (c = ' '; c <= '~'; c++)
	vl_chartypes_[c] |= vl_print;

    /* whitespace */
    vl_chartypes_[' '] |= vl_space;
#if OPT_ISO_8859
    vl_chartypes_[0xa0] |= vl_space;
#endif
    vl_chartypes_['\t'] |= vl_space;
    vl_chartypes_['\r'] |= vl_space;
    vl_chartypes_['\n'] |= vl_space;
    vl_chartypes_['\f'] |= vl_space;

#endif /* OPT_LOCALE */

    /* legal in pathnames */
    vl_chartypes_['.'] |= vl_pathn;
    vl_chartypes_['_'] |= vl_pathn;
    vl_chartypes_['~'] |= vl_pathn;
    vl_chartypes_['-'] |= vl_pathn;
    vl_chartypes_['*'] |= vl_pathn;
    vl_chartypes_['/'] |= vl_pathn;

    /* legal in "identifiers" */
    vl_chartypes_['_'] |= vl_ident | vl_qident;
    vl_chartypes_[':'] |= vl_qident;
#if SYS_VMS
    vl_chartypes_['$'] |= vl_ident | vl_qident;
#endif

    c = global_g_val(GVAL_PRINT_LOW);

    /*
     * Guard against setting printing-high before printing-low while we have a
     * buffer which may be repainted and possibly trashing the display.
     */
    if (c == 0
	&& global_g_val(GVAL_PRINT_HIGH) >= 254)
	c = 160;

    if (c < HIGHBIT)
	c = HIGHBIT;
    while (c <= global_g_val(GVAL_PRINT_HIGH) && c < N_chars)
	vl_chartypes_[c++] |= vl_print;

#if DISP_X11
    for (c = 0; c < N_chars; c++) {
	if (isPrint(c) && !gui_isprint(c)) {
	    vl_chartypes_[c] &= ~vl_print;
	}
    }
#endif
    /* backspacers: ^H, rubout */
    vl_chartypes_['\b'] |= vl_bspace;
    vl_chartypes_[127] |= vl_bspace;

    /* wildcard chars for most shells */
    vl_chartypes_['*'] |= vl_wild;
    vl_chartypes_['?'] |= vl_wild;
#if !OPT_VMS_PATH
#if SYS_UNIX
    vl_chartypes_['~'] |= vl_wild;
#endif
    vl_chartypes_[L_BLOCK] |= vl_wild;
    vl_chartypes_[R_BLOCK] |= vl_wild;
    vl_chartypes_[L_CURLY] |= vl_wild;
    vl_chartypes_[R_CURLY] |= vl_wild;
    vl_chartypes_['$'] |= vl_wild;
    vl_chartypes_['`'] |= vl_wild;
#endif

    /* ex mode line specifiers */
    vl_chartypes_[','] |= vl_linespec;
    vl_chartypes_['%'] |= vl_linespec;
    vl_chartypes_['-'] |= vl_linespec;
    vl_chartypes_['+'] |= vl_linespec;
    vl_chartypes_[';'] |= vl_linespec;
    vl_chartypes_['.'] |= vl_linespec;
    vl_chartypes_['$'] |= vl_linespec;
    vl_chartypes_['\''] |= vl_linespec;

    /* fences */
    vl_chartypes_[L_CURLY] |= vl_fence;
    vl_chartypes_[R_CURLY] |= vl_fence;
    vl_chartypes_[L_PAREN] |= vl_fence;
    vl_chartypes_[R_PAREN] |= vl_fence;
    vl_chartypes_[L_BLOCK] |= vl_fence;
    vl_chartypes_[R_BLOCK] |= vl_fence;

#if OPT_VMS_PATH
    vl_chartypes_[L_BLOCK] |= vl_pathn;		/* actually, "<", ">" too */
    vl_chartypes_[R_BLOCK] |= vl_pathn;
    vl_chartypes_['$'] |= vl_pathn;
    vl_chartypes_[':'] |= vl_pathn;
    vl_chartypes_[';'] |= vl_pathn;
#endif

#if OPT_MSDOS_PATH
    vl_chartypes_[BACKSLASH] |= vl_pathn;
    vl_chartypes_[':'] |= vl_pathn;
#endif

#if OPT_WIDE_CTYPES
    /* scratch-buffer-names (usually superset of vl_pathn) */
    vl_chartypes_[(unsigned) SCRTCH_LEFT[0]] |= vl_scrtch;
    vl_chartypes_[(unsigned) SCRTCH_RIGHT[0]] |= vl_scrtch;
    vl_chartypes_[' '] |= vl_scrtch;	/* ...to handle "[Buffer List]" */
#endif

    for (c = 0; c < N_chars; c++) {
	if (!(isSpace(c)))
	    vl_chartypes_[c] |= vl_nonspace;
	if (isDigit(c))
	    vl_chartypes_[c] |= vl_linespec;
	if (isAlpha(c) || isDigit(c))
	    vl_chartypes_[c] |= vl_ident | vl_pathn | vl_qident;
#if OPT_WIDE_CTYPES
	if (isSpace(c) || isPrint(c))
	    vl_chartypes_[c] |= vl_shpipe;
	if (ispath(c))
	    vl_chartypes_[c] |= vl_scrtch;
#endif
    }
    returnVoid();
}

#if OPT_HEAPSIZE

/* track overall heap usage */

#undef	realloc
#undef	malloc
#undef	free

long currentheap;		/* current heap usage */

typedef struct {
    size_t length;
    unsigned magic;
} HEAPSIZE;

#define MAGIC_HEAP 0x12345678

/* display the amount of HEAP currently malloc'ed */
static void
display_heap_usage(void)
{
    beginDisplay();
    if (global_g_val(GMDHEAPSIZE)) {
	char membuf[20];
	int saverow = ttrow;
	int savecol = ttcol;

	if (saverow >= 0 && saverow < term.rows
	    && savecol >= 0 && savecol < term.cols) {
	    (void) lsprintf(membuf, "[%ld]", currentheap);
	    kbd_overlay(membuf);
	    kbd_flush();
	    movecursor(saverow, savecol);
	}
    }
    endofDisplay();
}

static char *
old_ramsize(char *mp)
{
    HEAPSIZE *p = (HEAPSIZE *) (mp - sizeof(HEAPSIZE));

    if (p->magic == MAGIC_HEAP) {
	mp = (char *) p;
	currentheap -= p->length;
    }
    return mp;
}

static char *
new_ramsize(char *mp, unsigned nbytes)
{
    HEAPSIZE *p = (HEAPSIZE *) mp;
    if (p != 0) {
	p->length = nbytes;
	p->magic = MAGIC_HEAP;
	currentheap += nbytes;
	mp = (char *) ((long) p + sizeof(HEAPSIZE));
    }
    return mp;
}

/* track reallocs */
char *
track_realloc(char *p, unsigned size)
{
    if (!p)
	return track_malloc(size);

    size += sizeof(HEAPSIZE);
    p = new_ramsize(realloc(old_ramsize(p), size), size);
    display_heap_usage();
    return p;
}

/* track mallocs */
char *
track_malloc(
		unsigned size)
{
    char *p;

    size += sizeof(HEAPSIZE);
    if ((p = malloc(size)) != 0) {
	(void) memset(p, 0, size);	/* so we can use for calloc */
	p = new_ramsize(p, size);
	display_heap_usage();
    }

    return p;
}

/* track free'd memory */
void
track_free(char *p)
{
    if (!p)
	return;
    free(old_ramsize(p));
    display_heap_usage();
}
#endif /* OPT_HEAPSIZE */

#ifdef MALLOCDEBUG
mallocdbg(int f, int n)
{
    int lvl;
    lvl = malloc_debug(n);
    mlwrite("malloc debug level was %d", lvl);
    if (!f) {
	malloc_debug(lvl);
    } else if (n > 2) {
	malloc_verify();
    }
    return TRUE;
}
#endif

/*
 *	the log file is left open, unbuffered.  thus any code can do
 *
 *	extern FILE *FF;
 *	fprintf(FF, "...", ...);
 *
 *	to log events without disturbing the screen
 */

#ifdef DEBUGLOG
/* suppress the declaration so that the link will fail if someone uses it */
FILE *FF;
#endif

/*ARGSUSED*/
static void
start_debug_log(int ac GCC_UNUSED, char **av GCC_UNUSED)
{
#ifdef DEBUGLOG
    int i;
    FF = fopen("vilelog", "w");
    setbuf(FF, NULL);
    for (i = 0; i < ac; i++)
	(void) fprintf(FF, "arg %d: %s\n", i, av[i]);
#endif
}

#if SYS_MSDOS

#if CC_TURBO
int
showmemory(int f, int n)
{
    extern long coreleft(void);
    mlforce("Memory left: %ld bytes", coreleft());
    return TRUE;
}
#endif

#if CC_WATCOM
int
showmemory(int f, int n)
{
    mlforce("Watcom C doesn't provide a very useful 'memory-left' call.");
    return TRUE;
}
#endif

#if CC_DJGPP
int
showmemory(int f, int n)
{
    mlforce("Memory left: %ld Kb virtual, %ld Kb physical",
	    _go32_dpmi_remaining_virtual_memory() / 1024,
	    _go32_dpmi_remaining_physical_memory() / 1024);
    return TRUE;
}
#endif
#endif /* SYS_MSDOS */

char *
strncpy0(char *t, const char *f, size_t l)
{
    (void) strncpy(t, f, l);
    if (l)
	t[l - 1] = EOS;
    return t;
}

/*
 * This is probably more efficient for copying short strings into a large
 * fixed-size buffer, because strncpy always zero-pads the destination to
 * the given length.
 */
char *
vl_strncpy(char *dest, const char *src, size_t destlen)
{
    size_t srclen = strlen(src) + 1;
    if (srclen > destlen)
	srclen = destlen;
    return strncpy0(dest, src, srclen);
}

#if defined(SA_RESTART)
/* several systems (SCO, SunOS) have sigaction without SA_RESTART */
/*
 * Redefine signal in terms of sigaction for systems which have the
 * SA_RESTART flag defined through <signal.h>
 *
 * This definition of signal will cause system calls to get restarted for a
 * more BSD-ish behavior.  This will allow us to use the OPT_WORKING feature
 * for such systems.
 */

void
setup_handler(int sig, void (*disp) (int ACTUAL_SIG_ARGS))
{
    struct sigaction act, oact;

    act.sa_handler = disp;
    sigemptyset(&act.sa_mask);
#ifdef SA_NODEFER		/* don't rely on it.  if it's not there, signals
				   probably aren't deferred anyway. */
    act.sa_flags = SA_RESTART | SA_NODEFER;
#else
    act.sa_flags = SA_RESTART;
#endif

    (void) sigaction(sig, &act, &oact);

}

#else
void
setup_handler(int sig, void (*disp) (int ACTUAL_SIG_ARGS))
{
    (void) signal(sig, disp);
}
#endif

/* put us in a new process group, on command.  we don't do this all the
* time since it interferes with suspending xvile on some systems with some
* shells.  but we _want_ it other times, to better isolate us from signals,
* and isolate those around us (like buggy window/display managers) from
* _our_ signals.  so we punt, and leave it up to the user.
*/
/* ARGSUSED */
int
newprocessgroup(int f GCC_UNUSED, int n GCC_UNUSED)
{
#if DISP_X11

    int pid;

    if (f) {
#if SYS_VMS
	pid = vfork();		/* VAX C and DEC C have a 'vfork()' */
#else
	pid = fork();
#endif

	if (pid > 0)
	    ExitProgram(GOODEXIT);
	else if (pid < 0) {
	    fputs("cannot fork\n", stderr);
	    tidy_exit(BADEXIT);
	}
    }
# if !SYS_VMS
#  ifdef HAVE_SETSID
    (void) setsid();
#  else
#   ifdef HAVE_BSD_SETPGRP
    (void) setpgrp(0, 0);
#   else
    (void) setpgrp();
#   endif			/* HAVE_BSD_SETPGRP */
#  endif			/* HAVE_SETSID */
# endif				/* SYS_VMS */
#endif /* DISP_X11 */
    return TRUE;
}

static int
cmd_mouse_motion(const CMDFUNC * cfp)
{
    int result = FALSE;
#if (OPT_XTERM || DISP_X11)
    if (cfp != 0
	&& CMD_U_FUNC(cfp) == mouse_motion)
	result = TRUE;
#endif
    return result;
}

static void
make_startup_file(char *name)
{
    FILE *fp;
    char temp[NFILEN];

    if (strcmp(pathcat(temp, home_dir(), startup_file), startup_file)
	&& ((fp = fopen(temp, "w")) != 0)) {
	fprintf(fp, "; generated by %s -I\n", prog_arg);
	fprintf(fp, "source %s\n", name);
	fclose(fp);
    }
}

#ifdef VILE_ERROR_ABORT
#undef ExitProgram		/* in case it is oleauto_exit() */
void
ExitProgram(int code)
{
    char *env;
    if (code != GOODEXIT
	&& (env = getenv("VILE_ERROR_ABORT")) != 0
	&& *env != '\0')
	abort();
    exit(code);
}
#endif
