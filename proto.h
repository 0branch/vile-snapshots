/*
 *   This file was automatically generated by cextract version 1.2.
 *   Manual editing now recommended, since I've done a whole lot of it.
 *
 *   Created: Thu May 14 15:44:40 1992
 *
 * $Header: /users/source/archives/vile.vcs/RCS/proto.h,v 1.321 1998/12/17 03:12:42 tom Exp $
 *
 */

#ifndef VILE_PROTO_H
#define VILE_PROTO_H 1

#if !CHECK_PROTOTYPES
extern int main (int argc, char **argv);
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern SIGT catchintr (int ACTUAL_SIG_ARGS);
extern char *strncpy0 (char *t, const char *f, SIZE_T l);
extern int call_cmdfunc(const CMDFUNC *p, int f, int n);
extern int no_memory (const char *s);
extern int rdonly (void);
extern int writeall (int f, int n, int promptuser, int leaving, int autowriting);
extern void charinit (void);
extern void do_repeats (int *cp, int *fp, int *np);
extern void not_interrupted (void);
extern void setup_handler (int sig, void (*disp) (int ACTUAL_SIG_ARGS));
extern void tidy_exit (int code);

#ifndef interrupted
extern int interrupted (void);
#endif

#ifndef strmalloc
extern char *strmalloc (const char *s);
#endif

#if OPT_RAMSIZE
extern char *reallocate (char *mp, unsigned nbytes);
extern char *allocate (unsigned nbytes);
extern void release (char *mp);
#endif

/* screen-drivers */
#if OPT_XTERM >= 3
extern int xterm_mouse_t (int f, int n);
extern int xterm_mouse_T (int f, int n);
#endif

#if OPT_PERL
/* perl.xs (perl.c) */
extern	void	perl_default_region(void);
extern	void	perl_free_handle(void *);
extern	void	perl_free_callback(char *);
extern	void	perl_exit(void);
#if OPT_NAMEBST
extern	int	perl_call_sub(void *, int, int, int);
extern	void	perl_free_sub(void *);
#endif

/* api.c */
extern void api_free_private(void *);
/* There are others as well, but the rest are found in api.h */
#endif

/* basic.c */
extern int firstchar (LINEPTR lp);
extern int getgoal (LINEPTR dlp);
extern int gonmmark (int c);
extern int lastchar (LINEPTR lp);
extern int next_column (int c, int col);
extern int next_sw(int col);
extern int nextchar (LINEPTR lp, int off);
extern int setmark (void);
extern void swapmark (void);

#if OPT_MOUSE
extern	int setcursor (int row, int col);
extern	int setwmark (int row, int col);
#endif

#if SMALLER	/* cancel 'neproto.h' */
extern int gotobob (int f, int n);
extern int gotoeob (int f, int n);
#endif

/* bind.c */
extern char *flook (char *fname, UINT hflag);
extern char *kbd_engl (const char *prompt, char *buffer);
extern char *kcod2pstr (int c, char *seq);
extern const CMDFUNC *engl2fnc (const char *fname);
extern const CMDFUNC *kcod2fnc (int c);
extern int fnc2kcod (const CMDFUNC *);
extern int kbd_complete (int case_insensitive, int c, char *buf, unsigned *pos, const char *table, SIZE_T size_entry);
extern int kbd_engl_stat (const char *prompt, char *buffer, int stated);
extern int kbd_length (void);
extern int kcod2escape_seq (int c, char *ptr);
extern int no_such_function (const char *fname);
extern void kbd_alarm (void);
extern void kbd_erase (void);
extern void kbd_erase_to_end (int column);
extern void kbd_init (void);
extern void kbd_putc (int c);
extern void kbd_puts (const char *s);
extern void kbd_unquery (void);
extern void popdown_completions (void);

#if DISP_X11
extern char *fnc2pstr (const CMDFUNC *f);
#endif

#if OPT_EVAL
extern const char *prc2engl (const char *skey);
#endif

#if OPT_MENUS
extern char *give_accelerator ( char * );
#endif

#if OPT_NAMEBST
extern int delete_namebst(const char *name, int release);
extern int insert_namebst(const char *name, const CMDFUNC *cmd, int ro);
extern int rename_namebst(const char *oldname, const char *newname);
extern int search_namebst(const char *name);
extern void build_namebst(const NTAB *nametbl, int lo, int hi);
#endif

/* buffer.c */
BUFFER *make_bp (const char *fname, UINT flags);
extern BUFFER *bfind (const char *bname, UINT bflag);
extern BUFFER *find_alt (void);
extern BUFFER *find_any_buffer (const char *name);
extern BUFFER *find_b_file (const char *fname);
extern BUFFER *find_b_hist(int number);
extern BUFFER *find_b_name (const char *name);
extern BUFFER *find_bp (BUFFER *bp1);
extern BUFFER *getfile2bp (const char *fname, int ok_to_ask, int cmdline);
extern WINDOW *bp2any_wp (BUFFER *bp);
extern char *add_brackets(char *dst, const char *src);
extern char *hist_lookup ( int c );
extern char *strip_brackets(char *dst, const char *src);
extern int add_line_at (BUFFER *bp, LINEPTR prevp, const char *text, int len);
extern int addline (BUFFER *bp, const char *text, int len);
extern int any_changed_buf (BUFFER **bpp);
extern int any_unread_buf (BUFFER **bpp);
extern int bclear (BUFFER *bp);
extern int bsizes (BUFFER *bp);
extern int delink_bp (BUFFER *bp);
extern int popupbuff (BUFFER *bp);
extern int shiftwid_val (BUFFER *bp);
extern int swbuffer (BUFFER *bp);
extern int swbuffer_lfl (BUFFER *bp, int lockfl);
extern int tabstop_val (BUFFER *bp);
extern int zotbuf (BUFFER *bp);
extern int renamebuffer(BUFFER *rbp, char *bufname);
extern int zotwp (BUFFER *bp);
extern void chg_buff (BUFFER *bp, USHORT flag);
extern void imply_alt (char *fname, int copy, int lockfl);
extern void make_current (BUFFER *nbp);
extern void set_bname (BUFFER *bp, const char *name);
extern void sortlistbuffers (void);
extern void unchg_buff (BUFFER *bp, USHORT flag);
extern void undispbuff (BUFFER *bp, WINDOW *wp);

#if BEFORE
extern char *get_bname (BUFFER *bp);
#endif

#if !OPT_MAJORMODE
extern int has_C_suffix (BUFFER *bp);
#endif

#if OPT_UPBUFF
void updatelistbuffers (void);
void update_scratch (const char *name, int (*func)(BUFFER *));
#else
#define updatelistbuffers()
#define update_scratch(name, func)
#endif

/* crypt.c */
#if	OPT_ENCRYPT
extern	int	ue_makekey (char *key, UINT len);
extern	void	ue_crypt (char *bptr, UINT len);
#endif	/* OPT_ENCRYPT */

/* csrch.c */

/* display.c */
extern char *lsprintf (char *buf, const char *fmt, ...);
extern int col_limit (WINDOW *wp);
extern int im_waiting (int flag);
extern int mk_to_vcol (MARK mark, int expanded, int base);
extern int nu_width (WINDOW *wp);
extern int offs2col (WINDOW *wp, LINEPTR lp, C_NUM offset);
extern int update (int force);
extern int video_alloc (VIDEO **vpp);
extern int vtinit (void);
extern void bottomleft (void);
extern void bprintf (const char *fmt, ...);
extern void bputc (int c);
extern void dbgwrite (const char *fmt, ...);
extern void hilite (int row, int colfrom, int colto, int on);
extern void kbd_flush (void);
extern void kbd_openup (void);
extern void kbd_overlay(const char *s);
extern void mlerase (void);
extern void mlerror (const char *s);
extern void mlforce (const char *fmt, ...);
extern void mlprompt (const char *fmt, ...);
extern void mlsavec (int c);
extern void mlwarn (const char *fmt, ...);
extern void mlwrite (const char *fmt, ...);
extern void movecursor (int row, int col);
extern void newscreensize (int h, int w);
extern void upmode (void);

#if !DISP_X11
extern void getscreensize (int *widthp, int *heightp);
#if defined(SIGWINCH)
extern SIGT sizesignal (int ACTUAL_SIG_ARGS);
#endif
#endif

#if defined(SIGWINCH) || OPT_WORKING
extern void beginDisplay (void);
extern void endofDisplay (void);
#endif

#ifdef WMDLINEWRAP
extern int line_height (WINDOW *wp, LINEPTR lp);
#else
#define line_height(wp,lp) 1
#endif

#if defined(WMDLINEWRAP) || OPT_MOUSE
extern WINDOW *row2window (int row);
extern int col2offs (WINDOW *wp, LINEPTR lp, C_NUM col);
#endif

#if OPT_WORKING
extern SIGT imworking (int ACTUAL_SIG_ARGS);
#endif

#if OPT_PSCREEN
extern	OUTC_DCL psc_putchar	(OUTC_ARGS);
extern	void	psc_eeol	(void);
extern	void	psc_eeop	(void);
extern	void	psc_flush	(void);
extern	void	psc_move	(int row, int col);
extern	void	psc_rev		(UINT huh);
#endif	/* OPT_PSCREEN */

/* djhandl.c */
#if CC_DJGPP
extern unsigned long was_ctrl_c_hit (void);
extern void want_ctrl_c (int yes);
extern void clear_hard_error (void);
extern void hard_error_catch_setup (void);
extern void hard_error_teardown (void);
extern int did_hard_error_occur (void);
#endif

/* eval.c */
extern char *get_shell(void);
extern char *l_itoa (int i);
extern char *mklower (char *str);
extern char *mktrimmed (char *str);
extern const char * get_token (const char *src, char *tok, int eolchar);
extern const char * tokval (const char *tokn);
extern const char *skip_cblanks (const char *str);
extern const char *skip_cstring (const char *str);
extern const char *skip_ctext (const char *str);
extern int absol (int x);
extern int is_falsem (const char *val);
extern int is_truem (const char *val);
extern int mac_literalarg (TBUFF **tok);
extern int mac_token(char *tok);
extern int mac_tokval (char *tok);
extern int macroize (TBUFF **p, TBUFF *src, int skip);
extern int toktyp (const char *tokn);

#ifdef const
#define skip_blanks(s) skip_cblanks(s)
#define skip_string(s) skip_cstring(s)
#define skip_text(s) skip_ctext(s)
#else
extern char *skip_blanks (char *str);
extern char *skip_string (char *str);
extern char *skip_text (char *str);
#endif

#if OPT_EVAL
extern LINEPTR label2lp (BUFFER *bp, const char *label);
extern char *gtenv (const char *vname);
extern int   rmenv(const char *name);
extern int   set_variable (const char *name);
extern int   stenv(const char *name, const char *value);
#endif

#if OPT_EVAL || DISP_X11
extern int stol (const char *val);
#endif

#if OPT_EVAL || OPT_COLOR
extern int set_palette (const char *value);
#endif

#if OPT_EVAL || !SMALLER
extern char *mkupper (char *str);
#endif

#if OPT_COLOR
extern void set_ctrans (const char *value);
#endif

/* exec.c */
extern int do_source (char *fname, int n, int optional);
extern int dobuf (BUFFER *bp);
extern int docmd (char *cline, int execflag, int f, int n);
extern int dofile (char *fname);
extern int end_named_cmd (void);
extern int execute (const CMDFUNC *execfunc, int f, int n);
extern int more_named_cmd (void);

#if OPT_PROCEDURES
extern int run_procedure (const char *name);
#endif

/* file.c */
extern SIGT imdying (int ACTUAL_SIG_ARGS);
extern int bp2readin (BUFFER *bp, int lockfl);
extern int getfile (char *fname, int lockfl);
extern int ifile (char *fname, int belowthisline, FILE *haveffp);
extern int kwrite (char *fn, int msgf);
extern int no_file_found (void);
extern int no_such_file (const char *fname);
extern int readin (char *fname, int lockfl, BUFFER *bp, int mflg);
extern int same_fname (char *fname, BUFFER *bp, int lengthen);
extern int slowreadf (BUFFER *bp, int *nlinep);
extern int writeout (const char *fn, BUFFER *bp, int forced, int msgf);
extern int writeregion (void);
extern time_t file_modified (char *path);
extern void makename (char *bname, const char *fname);
extern void markWFMODE (BUFFER *bp);
extern void set_last_file_edited(const char *);
extern void unqname (char *name);

#ifdef MDCHK_MODTIME
extern int ask_shouldchange (BUFFER *bp);
extern int check_modtime (BUFFER *bp, char *fn);
extern int check_visible_modtimes (void);
extern int get_modtime (BUFFER *bp, time_t *the_time);
extern void set_modtime (BUFFER *bp, char *fn);
#endif

#if OPT_ENCRYPT
extern int resetkey (BUFFER *bp, const char *fname);
#endif

#if SMALLER	/* cancel neproto.h */
extern int filesave (int f, int n);
#endif

/* filec.c */
extern char *filec_expand (void);
extern int mlreply_dir (const char *prompt, TBUFF **buf, char *result);
extern int mlreply_file (const char *prompt, TBUFF **buf, UINT flag, char *result);

#if COMPLETE_FILES || COMPLETE_DIRS
extern int path_completion (DONE_ARGS);
extern void init_filec(const char *buffer_name);
#endif

/* fileio.c */
extern int ffaccess (char *fn, int mode);
extern int ffclose (void);
extern int ffexists (char *p);
extern int ffhasdata (void);
extern int ffputc (int c);
extern int ffputline (const char *buf, int nbuf, const char *ending);
extern int ffronly (char *fn);
extern int ffropen (char *fn);
extern int ffwopen (char *fn, int forced);
extern B_COUNT ffsize (void);

#if !(SYS_MSDOS || SYS_WIN31)
extern int ffread (char *buf, long len);
extern void ffseek (long n);
extern void ffrewind (void);
#endif

/* finderr.c */
#if OPT_FINDERR
extern void set_febuff (const char *name);
#endif

/* glob.c */
extern	char **	glob_free   (char **list_of_items);
extern	char **	glob_string (char *item);
extern	int	doglob (char *path);
extern	int	glob_length (char **list_of_items);

#if !SYS_UNIX
extern	int	glob_needed (char **list_of_items);
extern	void	expand_wild_args (int *argcp, char ***argvp);
#endif

/* globals.c */

/* history.c */
#if OPT_HISTORY
extern	int	edithistory (TBUFF **buffer, unsigned *position, int *given, UINT options, int (*func)(EOL_ARGS), int eolchar);
extern	void	hst_append (TBUFF *cmd, int glue);
extern	void	hst_append_s (char *cmd, int glue);
extern	void	hst_flush (void);
extern	void	hst_glue (int c);
extern	void	hst_init (int c);
extern	void	hst_remove (const char *cmd);
#else
#define	hst_append(p,c)
#define	hst_append_s(p,c)
#define	hst_flush()
#define	hst_glue(c)
#define	hst_init(c)
#define	hst_remove(p)
#endif

/* ibmpc.c */
#if SYS_MSDOS || SYS_OS2 || SYS_WINNT
extern	VIDEO *	scread (VIDEO *vp, int row);
extern	void	scwrite (int row, int col, int nchar, const char *outstr, VIDEO_ATTR *attrstr, int forg, int bacg);
#endif

/* input.c */
extern int dotcmdbegin (void);
extern int dotcmdfinish (void);
extern int end_string (void);
extern int eol_history(EOL_ARGS);
extern int get_recorded_char (int eatit);
extern int is_edit_char (int c);
extern int kbd_delimiter (void);
extern int kbd_is_pushed_back (void);
extern int kbd_replaying (int match);
extern int kbd_reply (const char *prompt, TBUFF **extbuf, int (*efunc)(EOL_ARGS), int eolchar, UINT options, int (*cfunc)(DONE_ARGS));
extern int kbd_seq (void);
extern int kbd_seq_nomap (void);
extern int kbd_show_response (TBUFF **dst, char *src, unsigned bufn, int eolchar, UINT options);
extern int kbd_string (const char *prompt, char *extbuf, unsigned bufn, int eolchar, UINT options, int (*func)(DONE_ARGS));
extern int kbm_started (int macnum, int force);
extern int keystroke (void);
extern int keystroke8 (void);
extern int keystroke_avail (void);
extern int keystroke_raw8 (void);
extern int mapped_keystroke (void);
extern int mlquickask (const char *prompt, const char *respchars, int *cp);
extern int mlreply (const char *prompt, char *buf, UINT bufn);
extern int mlreply_no_bs (const char *prompt, char *buf, UINT bufn);
extern int mlreply_no_opts (const char *prompt, char *buf, UINT bufn);
extern int mlreply_reg (const char *prompt, char *cbuf, int *retp, int at_dft);
extern int mlreply_reg_count (int state, int *retp, int *next);
extern int mlyesno (const char *prompt);
extern int no_completion (DONE_ARGS);
extern int screen_string (char *buf, int bufn, CHARTYPE inclchartype);
extern int start_kbm (int n, int macnum, ITBUFF *ptr);
extern int tgetc (int quoted);
extern int tgetc_avail (void);
extern void dotcmdstop (void);
extern void incr_dot_kregnum (void);
extern void kbd_kill_response (TBUFF *buf, unsigned *position, int c);
extern void kbd_pushback (TBUFF *buf, int skip);
extern void set_end_string (int c);
extern void tungetc(int c);
extern void unkeystroke (int c);

#if COMPLETE_FILES
extern int shell_complete (DONE_ARGS);
#else
#define shell_complete no_completion
#endif

/* insert.c */
extern int indentlen (LINEPTR lp);
extern int ins (void);
extern int ins_mode (WINDOW *wp);
extern int inschar (int c, int *backsp_limit_p);
extern int previndent (int *bracefp);

#if OPT_EVAL
extern char *current_modename (void);
#endif

#if SMALLER	/* cancel 'neproto.h' */
extern int newline (int f, int n);
extern int wrapword (int f, int n);
#endif

/* isearch.c */
#if SMALLER	/* cancel 'neproto.h' */
extern int forwhunt (int f, int n);
extern int backhunt (int f, int n);
#endif

/* itbuff.c */
ALLOC_T	 itb_length (ITBUFF *p);
ITBUFF * itb_alloc (ITBUFF **p, ALLOC_T n);
ITBUFF * itb_append (ITBUFF **p, int c);
ITBUFF * itb_bappend (ITBUFF **, const char *s, ALLOC_T len);
ITBUFF * itb_copy (ITBUFF **d, ITBUFF *s);
ITBUFF * itb_init (ITBUFF **p, int c);
ITBUFF * itb_sappend (ITBUFF **, const char *s);
int	 itb_get (ITBUFF *p, ALLOC_T n);
int	 itb_last (ITBUFF *p);
int	 itb_more (ITBUFF *p);
int	 itb_next (ITBUFF *p);
int	 itb_peek (ITBUFF *p);
int *	 itb_values (ITBUFF *p);
void	 itb_first (ITBUFF *p);
void	 itb_free (ITBUFF **p);
void	 itb_stuff (ITBUFF *p, int c);

#if NEEDED
ITBUFF * itb_insert (ITBUFF **p, int c);
void	 itb_delete (ITBUFF *p, ALLOC_T cnt);
void	 itb_unnext (ITBUFF *p);
void	 itb_unput (ITBUFF *p);
#endif

/* lckfiles.c */
#if OPT_LCKFILES
extern int set_lock (const char *fname, char *who, int n);
extern void release_lock (const char *fname);
#endif

/* line.c */
extern LINEPTR lalloc (int used, BUFFER *bp);
extern int begin_kill (void);
extern int do_report (L_NUM value);
extern int index2reg (int c);
extern int index2ukb (int inx);
extern int kinsert (int c);
extern int kinsertlater (int c);
extern int ldelete (long n, int kflag);
extern int linsert (int n, int c);
extern int lnewline (void);
extern int lstrinsert (const char *s, int len);
extern int reg2index (int c);
extern void end_kill (void);
extern void kdone (void);
extern void kregcirculate (int killing);
extern void ksetup (void);
extern void lfree (LINEPTR lp, BUFFER *bp);
extern void lremove (BUFFER *bp, LINEPTR lp);
extern void ltextfree (LINEPTR lp, BUFFER *bp);

#if OPT_EVAL
extern char * getctext (CHARTYPE type);
extern int putctext (CHARTYPE type, const char *iline);
#endif

#if SMALLER	/* cancel neproto.h */
extern int insspace (int f, int n);
#endif

/* map.c */
extern int mapped_c (int remap, int raw);
extern int mapped_c_avail (void);
extern int mapped_ungotc_avail(void);
extern int sysmapped_c (void);
extern int sysmapped_c_avail (void);
extern void abbr_check (int *backsp_limit_p);
extern void addtosysmap (const char *seq, int seqlen, int code);
extern void mapungetc (int c);

/* menu.c */
#if OPT_MENUS
extern int parse_menu (const char *rc_filename);
#if NEED_X_INCLUDES
extern void do_menu ( Widget menub );
#endif
#endif

/* msgs.c */
#if OPT_POPUP_MSGS
void msg_putc (int c);
void popup_msgs (void);
void purge_msgs (void);
#endif

/* modes.c */
extern REGEXVAL *new_regexval (const char *pattern, int magic);
extern char *string_mode_val (VALARGS *args);
extern int adjvalueset (const char *cp, int setting, int global, VALARGS *args);
extern int find_mode (const char *mode, int global, VALARGS *args);
extern int mode_eol (EOL_ARGS);
extern int set_mode_value(const char *cp, int setting, int global, VALARGS *args, const char *rp);
extern int string_to_number (const char *from, int *np);
extern void copy_mvals (int maximum, struct VAL *dst, struct VAL *src);
extern void free_local_vals (const struct VALNAMES *names, struct VAL *gbl, struct VAL *val);

#if OPT_EVAL || OPT_COLOR
extern int set_ncolors(int ncolors);
#endif

#if OPT_EVAL || OPT_MAJORMODE
extern int is_varmode ( const char *name );
extern const char *const * list_of_modes (void);
#endif

#if OPT_MAJORMODE
extern int alloc_mode(const char *name, int predef);
extern void set_majormode_rexp(const char *name, int n, const char *pat);
extern void set_submode_val(const char *name, int n, int value);
extern void setm_by_suffix (BUFFER *bp);
extern void setm_by_preamble (BUFFER *bp);
#else
#define setm_by_suffix(bp) fix_cmode(bp, (global_b_val(MDCMOD) && has_C_suffix(bp)))
#define setm_by_preamble(bp) /* nothing */
#endif

#if OPT_UPBUFF
extern void save_vals (int maximum, struct VAL *gbl, struct VAL *dst, struct VAL *src);
#endif

#if SYS_VMS
extern const char *vms_record_format(int code);
#endif

/* npopen.c */
#if SYS_UNIX || SYS_MSDOS || SYS_WIN31 || SYS_OS2 || SYS_WINNT
extern FILE * npopen (char *cmd, const char *type);
extern void npclose (FILE *fp);
extern int inout_popen (FILE **fr, FILE **fw, char *cmd);
extern int softfork (void);
#endif

#if SYS_UNIX || SYS_OS2 || SYS_WINNT
extern int system_SHELL (char *cmd);
#endif

#if SYS_MSDOS || SYS_WIN31 || SYS_WINNT || (SYS_OS2 && CC_CSETPP) || TEST_DOS_PIPES
extern void npflush (void);
#endif

/* ntconio.c */
#if DISP_NTCONS
extern void ntcons_reopen(void);
#endif

/* oneliner.c */
extern int llineregion (void);
extern int plineregion (void);
extern int pplineregion (void);
extern int subst_again_region (void);
extern int substregion (void);

/* opers.c */
extern int vile_op (int f, int n, OpsFunc fn, const char *str);

/* path.c */
extern char * is_appendname (char *fn);
extern char * last_slash (char *fn);
extern char * lengthen_path (char *path);
extern char * pathcat (char *dst, const char *path, char *leaf);
extern char * pathleaf (char *path);
extern char * shorten_path (char *path, int keep_cwd);
extern int is_directory (char *path);
extern int is_internalname (const char *fn);
extern int is_pathname (char *path);
extern int is_scratchname (const char *fn);
extern int maybe_pathname (char *fn);

#if OPT_MSDOS_PATH
extern char * is_msdos_drive (char *path);
extern char * sl_to_bsl (const char *p);
#endif

#ifndef bsl_to_sl_inplace
extern void bsl_to_sl_inplace (char *p);
#endif

#if OPT_PATHLOOKUP && (SYS_UNIX||SYS_VMS||OPT_MSDOS_PATH)
extern const char *parse_pathlist (const char *list, char *result);
#endif

#if OPT_CASELESS && SYS_OS2
extern int is_case_preserving (const char *name);
#endif

#if OPT_VMS_PATH
extern char * strip_version (char *path);
extern char * unix_pathleaf (char *path);
extern char * vms_pathleaf (char *path);
extern int is_vms_pathname (const char *path, int option);
#endif

#if SYS_UNIX || !SMALLER
extern char * home_path (char *path);
#endif

/* random.c */
extern L_NUM line_count (BUFFER *the_buffer);
extern L_NUM line_no (BUFFER *the_buffer, LINEPTR the_line);
extern char * current_directory (int force);
extern int fmatchindent (int c);
extern int getccol (int bflg);
extern int getcol (MARK mark, int actual);
extern int getoff (C_NUM goal, C_NUM *rcolp);
extern int gocol (int n);
extern int is_user_fence (int ch, int *sdirp);
extern int line_report (L_NUM before);
extern int liststuff (const char *name, int appendit, void (*)(LIST_ARGS), int iarg, void *vargp);
extern int set_directory (const char *dir);
extern void catnap (int milli, int watchinput);
extern void ch_fname (BUFFER *bp, const char *fname);
extern void set_rdonly (BUFFER *bp, const char *name, int mode);

#if OPT_EVAL
extern L_NUM getcline (void);
extern char * previous_directory (void);
#endif

#if SYS_MSDOS || SYS_OS2 || SYS_WINNT
extern const char * curr_dir_on_drive (int drive);
extern int curdrive (void);
extern int setdrive (int d);
extern void update_dos_drv_dir (char * cwd);
# if CC_WATCOM
     extern int dos_crit_handler (unsigned deverror, unsigned errcode, unsigned *devhdr);
# else
     extern void dos_crit_handler (void);
# endif
# if OPT_MS_MOUSE
     extern int ms_exists (void);
     extern void ms_processing (void);
# endif
#endif

/* regexp.c */
extern regexp * regcomp (char *origexp, int magic);
extern int regexec (regexp *prog, char *string, char *stringend, int startoff, int endoff);
extern int lregexec (regexp *prog, LINEPTR lp, int startoff, int endoff);

/* region.c */
typedef int (*DORGNLINES)(int (*)(REGN_ARGS), void *, int);

extern int        blank_region (void);
extern int        cryptregion (void);
extern int        detab_region (void);
extern int        detabline (void *flagp, int l, int r);
extern int        entab_region (void);
extern int        entabline (void *flagp, int l, int r);
extern int        flipregion (void);
extern DORGNLINES get_do_lines_rgn(void);
extern int        get_fl_region (REGION *rp);
extern int        getregion (REGION *rp);
extern int        killregion (void);
extern int        killregionmaybesave (int save);
extern int        lowerregion (void);
extern int        openregion (void);
extern int        shiftlregion (void);
extern int        shiftrregion (void);
extern int        stringrect (void);
extern int        trim_region (void);
extern int        trimline (void *flagp, int l, int r);
extern int        upperregion (void);
extern int        yankregion (void);

#if NEEDED
extern int charprocreg (int (*)(int));
#endif

/* search.c */
extern int findpat (int f, int n, regexp *exp, int direc);
extern int fsearch (int f, int n, int marking, int fromscreen);
extern int readpattern (const char *prompt, char *apat, regexp **srchexpp, int c, int fromscreen);
extern int scanner (regexp *exp, int direct, int wrapok, int *wrappedp);
extern void attrib_matches (void);
extern void regerror (const char *s);
extern void scanboundry (int wrapok, MARK dot, int dir);

#if OPT_EVAL || UNUSED
extern int eq (int bc, int pc);
#endif

#if OPT_HILITEMATCH
void clobber_save_curbp(BUFFER *bp);
#endif

/* select.c */
#if OPT_SELECTIONS
extern	LINEPTR setup_region    (void);
extern	int	assign_attr_id	(void);
extern	int	attribute_cntl_a_seqs_in_region(REGION *rp, REGIONSHAPE shape);
extern	int	attributeregion (void);
extern	int	attributeregion_in_region(REGION *rp, REGIONSHAPE shape,
			                    VIDEO_ATTR vattr, char *hc);
extern	int	sel_begin	(void);
extern	int	sel_extend	(int wiping, int include_dot);
extern	int	sel_setshape	(REGIONSHAPE shape);
extern	void	find_release_attr (BUFFER *bp, REGION *rp);
extern	void	free_attrib	(BUFFER *bp, AREGION *ap);
extern	void	free_attribs	(BUFFER *bp);
extern	void	sel_reassert_ownership (BUFFER *bp);
extern	void	sel_release	(void);
#if OPT_MOUSE
extern	int	on_mouse_click	(int button, int y, int x);
#endif
#if OPT_PERL || OPT_TCL
extern	BUFFER *get_selection_buffer_and_region(AREGION *arp);
#endif /* OPT_PERL || OPT_TCL */
#if OPT_SEL_YANK
extern	int	sel_yank	(int reg);
extern	int	sel_attached	(void);
extern	BUFFER *sel_buffer	(void);
#endif
#endif /* OPT_SELECTIONS */

/* spawn.c */
#if OPT_SHELL
extern SIGT rtfrmshell (int ACTUAL_SIG_ARGS);
extern void pressreturn (void);
extern int filterregion (void);
extern int open_region_filter (void);
#else
#define pressreturn() (void)keystroke()
#endif

/* tags.c */
#if OPT_TAGS
extern int cmdlinetag (const char *t);
#endif /* OPT_TAGS */

/* tbuff.c */
ALLOC_T	tb_length (TBUFF *p);
TBUFF *	tb_alloc (TBUFF **p, ALLOC_T n);
TBUFF *	tb_append (TBUFF **p, int c);
TBUFF *	tb_bappend (TBUFF **p, const char *s, ALLOC_T len);
TBUFF *	tb_copy (TBUFF **d, TBUFF *s);
TBUFF *	tb_init (TBUFF **p, int c);
TBUFF *	tb_insert (TBUFF **p, ALLOC_T n, int c);
TBUFF *	tb_put (TBUFF **p, ALLOC_T n, int c);
TBUFF *	tb_sappend (TBUFF **p, const char *s);
TBUFF *	tb_scopy (TBUFF **p, const char *s);
TBUFF *	tb_string (const char *s);
char *	tb_values (TBUFF *p);
int	tb_get (TBUFF *p, ALLOC_T n);
int	tb_more (TBUFF *p);
int	tb_next (TBUFF *p);
int	tb_peek (TBUFF *p);
void	tb_first (TBUFF *p);
void	tb_free (TBUFF **p);
void	tb_stuff (TBUFF *p, int c);
void	tb_unnext (TBUFF *p);
void	tb_unput (TBUFF *p);

/* termio.c */
extern OUTC_DCL ttputc (OUTC_ARGS);
extern int  null_t_watchfd (int fd, WATCHTYPE type, long *idp);
extern int  open_terminal (TERM *termp);
extern int  ttgetc (void);
extern int  tttypahead (void);
extern void null_t_cursor (int flag);
extern void null_t_icursor (int c);
extern void null_t_pflush (void);
extern void null_t_scroll (int f, int t, int n);
extern void null_t_setback (int b);
extern void null_t_setfor (int f);
extern void null_t_setpal (const char *p);
extern void null_t_title (char *t);
extern void null_t_unwatchfd (int fd, long id);
extern void ttclean (int f);
extern void ttclose (void);
extern void ttflush (void);
extern void ttopen (void);
extern void ttunclean (void);

/* undo.c */
extern void copy_for_undo (LINEPTR lp);
extern void dumpuline (LINEPTR lp);
extern void freeundostacks (BUFFER *bp, int both);
extern void mayneedundo (void);
extern void nounmodifiable (BUFFER *bp);
extern void tag_for_undo (LINEPTR lp);
extern void toss_to_undo (LINEPTR lp);

/* version.c */
extern const char * getversion (void);
extern const char * non_filename (void);
extern void print_usage (void);

/* vms2unix.c */
#if OPT_VMS_PATH
extern	char *	is_vms_dirtype	(char *path);
extern	char *	is_vms_rootdir	(char *path);
extern	char *	unix2vms_path   (char *dst, const char *src);
extern	char *	vms2unix_path   (char *dst, const char *src);
extern	char *	vms_path2dir    (const char *src);
extern	void	vms_dir2path	(char *path);
#endif

#if SYS_VMS
extern	int	vms_creat	(char *path);
#endif

/* vmspipe.c */
#if SYS_VMS
extern FILE *vms_rpipe (const char *cmd, int fd, const char *input_file);
#endif

/* w32isms */
#if SYS_WINNT

#define W32_SYS_ERROR  NOERROR

typedef struct fontstr_options_struct
{
    unsigned long size;      /* Font's point size.                     */
    int           bold;      /* Boolean -> T, user wants bold weight.  */
    int           italic;    /* Boolean -> T, user wants italic style. */
    char          face[256]; /* Font face requested by user.  If no face
                              * specified, face[0] == '\0';
                              */
} FONTSTR_OPTIONS;

typedef struct oleauto_options_struct
{
    int  invisible;     /* Boolean, T -> server invisible at startup        */
    int  multiple;      /* Boolean, T -> multiple server instances possible */
    int  rows, cols;    /* Size of winvile at startup, 0 if unspecified     */
    char *fontstr;      /* Ptr to font specification (see parse_font_str()),
                         * fall back on default font if NULL.
                         */
} OLEAUTO_OPTIONS;

extern void disp_win32_error(ULONG errcode, void *hwnd);
extern char *fmt_win32_error(ULONG errcode, char **buf, ULONG buflen);
extern int  is_win95(void);
extern int  is_winnt(void);
extern char *mk_shell_cmd_str(char *cmd, int *allocd_mem, int prepend_shc);
extern char *ntwinio_current_font(void);
extern int  ntwinio_font_frm_str(const char *fontstr, int use_mb);
extern void ntwinio_oleauto_reg(void);
extern void ntwinio_redirect_hwnd(int redirect);
extern void oleauto_exit(int code);
extern int  oleauto_init(OLEAUTO_OPTIONS *opts);
extern int  oleauto_redirected_key(ULONG keycode, ULONG keystate);
extern int  oleauto_register(OLEAUTO_OPTIONS *opts);
extern int  oleauto_unregister(void);
extern int  parse_font_str(const char *fontstr, FONTSTR_OPTIONS *results);
extern void restore_console_title(void);
extern void set_console_title(const char *title);
extern int  stdin_data_available(void);
extern int  w32_inout_popen(FILE **fr, FILE **fw, char *cmd);
extern void w32_keybrd_reopen(int pressret);
extern void w32_npclose(FILE *fp);
extern int  w32_system(const char *cmd);
extern int  w32_system_winvile(const char *cmd, int pressret);
extern char *w32_wdw_title();
extern void *winvile_hwnd(void);
#endif

/* watchfd.c */
extern int watchfd(int fd, WATCHTYPE type, char *callback);
extern void unwatchfd(int fd);
extern void dowatchcallback(int fd);

/* window.c */
extern WINDOW * wpopup (void);
extern int delwp (WINDOW *thewp);
extern int set_curwp (WINDOW *wp);
extern void copy_traits (W_TRAITS *dst, W_TRAITS *src);
extern void shrinkwrap (void);
extern void winit (int screen);

#if OPT_EVAL
extern int getwpos (void);
#endif

#if SMALLER	/* cancel neproto.h */
extern int reposition (int f, int n);
extern int resize (int f, int n);
#endif

#if OPT_SEL_YANK || OPT_PERL
extern WINDOW * push_fake_win(BUFFER *bp);
extern BUFFER * pop_fake_win(WINDOW *oldwp);
#endif

/* word.c */
extern int ffgetline (int *lenp);
extern int formatregion (void);
extern int isendviwordf (void);
extern int isendwordf (void);
extern int isnewviwordb (void);
extern int isnewviwordf (void);
extern int isnewwordb (void);
extern int isnewwordf (void);
extern int joinregion (void);
extern void fmatch (int rch);
extern void setchartype (void);

/* x11.c */
#if DISP_X11
extern	char *	x_current_fontname	(void);
extern	char *	x_get_icon_name (void);
extern	char *	x_get_window_name (void);
extern	int	x_setfont		(const char *fname);
extern	int	x_typahead		(int milli);
extern	void	x_move_events		(void);
extern	void	x_preparse_args		(int *pargc, char ***pargv);
extern	void	x_set_icon_name (const char *name);
extern	void	x_set_window_name (const char *name);

#if XTOOLKIT
extern	void	own_selection		(void);
#else	/* !XTOOLKIT */
extern	void	x_set_rv		(void);
extern	void	x_setname		(char *name);
extern	void	x_set_wm_title		(char *name);
extern	void	x_setforeground		(char *colorname);
extern	void	x_setbackground		(char *colorname);
extern  void	x_set_geometry		(char *g);
extern	void	x_set_dpy		(char *dn);
extern	int	x_key_events_ready	(void);
#endif	/* !XTOOKIT */

#if OPT_MENUS
extern	int	x_menu_height		(void);
#endif

#if OPT_MENUS_COLORED
extern	int	x_menu_background	(void);
extern	int	x_menu_foreground	(void);
#endif /* OPT_MENUS_COLORED */

#if OPT_WORKING
extern	void	x_working		(void);
#endif

extern	int	gui_isprint		(int ch);
#endif	/* DISP_X11 */

#if DISP_X11 || DISP_NTWIN
extern	void	gui_resize		(int cols, int rows);
#endif

#if DISP_NTWIN
extern	void	gui_version(char *program);
extern	void	gui_usage(char *program, const char *const *options, size_t length);
#endif

#if OPT_SCROLLBARS
extern	void	gui_update_scrollbar	(WINDOW *uwp);
#endif

#if NO_LEAKS
extern	void	bind_leaks (void);
extern	void	bp_leaks (void);
extern	void	ev_leaks (void);
extern	void	fileio_leaks (void);
extern	void	itb_leaks (void);
extern	void	kbs_leaks (void);
extern	void	map_leaks (void);
extern	void	mode_leaks (void);
extern	void	onel_leaks (void);
extern	void	path_leaks (void);
extern	void	tags_leaks (void);
extern	void	tb_leaks (void);
extern	void	vt_leaks (void);
extern	void	wp_leaks (void);

#if DISP_X11
extern	void	x11_leaks		(void);
#endif

#endif /* NO_LEAKS */

#if SYS_UNIX
#if MISSING_EXTERN__FILBUF
extern	int	_filbuf	(FILE *fp);
#endif
#if MISSING_EXTERN__FLSBUF
extern	int	_flsbuf	(int n, FILE *fp);
#endif
#if MISSING_EXTERN_ACCESS
extern	int	access	(const char *path, int mode);
#endif
#if MISSING_EXTERN_ALARM
extern	UINT	alarm	(UINT secs);
#endif
#if MISSING_EXTERN_ATOI
extern int	atoi	(const char *s);
#endif
#if MISSING_EXTERN_BZERO
extern	void	bzero	(char *b, int n);
#endif
#if MISSING_EXTERN_CHDIR
extern	int	chdir	(const char *path);
#endif
#if MISSING_EXTERN_CLOSE
extern	int	close	(int fd);
#endif
#if MISSING_EXTERN_DUP
extern	int	dup	(int fd);
#endif
#if MISSING_EXTERN_EXECLP
extern	int	execlp	(const char *path, ...);
#endif
#if MISSING_EXTERN_FCLOSE
extern	int	fclose	(FILE *fp);
#endif
#if MISSING_EXTERN_FCLOSE
extern	int	fflush	(FILE *fp);
#endif
#if MISSING_EXTERN_FGETC
extern	int	fgetc	(FILE *fp);
#endif
#if MISSING_EXTERN_FILENO && !defined(fileno)
extern	int	fileno	(FILE *fp);
#endif
#if MISSING_EXTERN_FORK
extern	int	fork	(void);
#endif
#if MISSING_EXTERN_FPRINTF
extern	int	fprintf	(FILE *fp, const char *fmt, ...);
#endif
#if MISSING_EXTERN_FPUTC
extern	int	fputc	(int c, FILE *fp);
#endif
#if MISSING_EXTERN_FPUTS
extern	int	fputs	(const char *s, FILE *fp);
#endif
#if MISSING_EXTERN_FREAD
extern	int	fread	(char *ptr, SIZE_T size, SIZE_T nmemb, FILE *fp);
#endif
#if MISSING_EXTERN_FREE
extern void	free	(void *ptr);
#endif
#if MISSING_EXTERN_FSEEK
extern	int	fseek	(FILE *fp, long offset, int whence);
#endif
#if MISSING_EXTERN_FWRITE
extern	int	fwrite	(const char *ptr, SIZE_T size, SIZE_T nmemb, FILE *fp);
#endif
#if MISSING_EXTERN_GETCWD && HAVE_GETCWD
extern	char *	getcwd (char *buffer, int len);
#endif
#if MISSING_EXTERN_GETENV
extern	char *	getenv	(const char *name);
#endif
#if MISSING_EXTERN_GETPASS
extern	char *	getpass	(const char *prompt);
#endif
#if MISSING_EXTERN_GETHOSTNAME && HAVE_GETHOSTNAME
extern	int	gethostname (char *name, int len);
#endif
#if MISSING_EXTERN_GETPGRP
extern	int	getpgrp	(int pid);
#endif
#if MISSING_EXTERN_GETPID
extern	int	getpid	(void);
#endif
#if MISSING_EXTERN_GETUID
extern	int	getuid	(void);
#endif
#if MISSING_EXTERN_GETWD && HAVE_GETWD
extern	char *	getwd	(char *buffer);
#endif
#if MISSING_EXTERN_IOCTL
extern	int	ioctl	(int fd, ULONG mask, caddr_t ptr);
#endif
#if MISSING_EXTERN_ISATTY
extern	int	isatty	(int fd);
#endif
#if MISSING_EXTERN_KILL
extern	int	kill	(int pid, int sig);
#endif
#if MISSING_EXTERN_KILLPG && HAVE_KILLPG
extern	int	killpg	(int pgrp, int sig);
#endif
#if MISSING_EXTERN_LINK && HAVE_LINK
extern	int	link	(const char *, const char *);
#endif
#if MISSING_EXTERN_LONGJMP
extern	void	longjmp	(jmp_buf env, int val);
#endif
#if MISSING_EXTERN_LSTAT
extern	int	lstat (const char *path, struct stat *sb);
#endif
#if MISSING_EXTERN_MEMSET
#ifndef memset	/* may be defined by dbmalloc */
extern	void *	memset	(void *dst, int ch, size_t n);
#endif
#endif
#if MISSING_EXTERN_MKDIR && HAVE_MKDIR
extern	int	mkdir	(const char *path, int mode);
#endif
#if MISSING_EXTERN_MKTEMP
extern	char *	mktemp (char *template);
#endif
#if MISSING_EXTERN_OPEN
extern	int	open	(char *path, int flags);
#endif
#if MISSING_EXTERN_PERROR
extern	void	perror	(const char *s);
#endif
#if MISSING_EXTERN_PIPE
extern	int	pipe	(int filedes[2]);
#endif
#if MISSING_EXTERN_PRINTF
extern	int	printf	(const char *fmt, ...);
#endif
#if MISSING_EXTERN_PUTS
extern	int	puts	(const char *s);
#endif
#if MISSING_EXTERN_QSORT
#if ANSI_QSORT
extern void qsort (void *base, size_t nmemb, size_t size, int (*compar)(const void *a, const void *b);
#else
extern void qsort (void *base, size_t nmemb, size_t size, int (*compar)(char **a, char **b);
#endif
#endif
#if MISSING_EXTERN_READ
extern	int	read	(int fd, char *buffer, SIZE_T size);
#endif
#if MISSING_EXTERN_READLINK
extern	int	readlink (const char *path, char *buffer, size_t size);
#endif
#if MISSING_EXTERN_SELECT && HAVE_SELECT
extern	int	select	(int numfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
#endif
#if MISSING_EXTERN_SETBUF
extern	void	setbuf	(FILE *fp, char *buffer);
#endif
#if MISSING_EXTERN_SETBUFFER
extern	void	setbuffer (FILE *fp, char *buffer, int size);
#endif
#if MISSING_EXTERN_SETITIMER && HAVE_SETITIMER
extern	int	setitimer (int which, const struct itimerval *value, struct itimerval *ovalue);
#endif
#if MISSING_EXTERN_SETJMP && !defined(setjmp)
extern	int	setjmp	(jmp_buf env);
#endif
#if MISSING_EXTERN_SETPGRP
#if SETPGRP_VOID
extern	pid_t	setpgrp	(void);
#else
extern	int	setpgrp	(int pid, int pgid);
#endif
#endif
#if MISSING_EXTERN_SETSID
extern	pid_t	setsid	(void);
#endif
#if MISSING_EXTERN_SETVBUF
#if SETVBUF_REVERSED
extern	int	setvbuf (FILE *fp, int mode, char *buffer, size_t size);
#else
extern	int	setvbuf (FILE *fp, char *buffer, int mode, size_t size);
#endif
#endif
#if MISSING_EXTERN_SLEEP
extern	int	sleep	(UINT secs);
#endif
#if MISSING_EXTERN_SSCANF
extern	int	sscanf	(const char *src, const char *fmt, ...);
#endif
#if MISSING_EXTERN_STRERROR
extern	char *	strerror (int code);
#endif
#if MISSING_EXTERN_STRTOL
extern	long	strtol	(const char *nptr, char **endptr, int base);
#endif
#if MISSING_EXTERN_SYSTEM
extern	int	system	(const char *cmd);
#endif
#if MISSING_EXTERN_TIME
extern	time_t	time	(time_t *t);
#endif
#if MISSING_EXTERN_UNLINK
extern	int	unlink	(char *path);
#endif
#if MISSING_EXTERN_UTIME && HAVE_UTIME
extern	int	utime	(const char *path, const struct utimbuf *t);
#endif
#if MISSING_EXTERN_UTIMES && HAVE_UTIMES
extern	int	utimes	(const char *path, struct timeval *t);
#endif
#if MISSING_EXTERN_WAIT
extern	int	wait	(int *sb);
#endif
#if MISSING_EXTERN_WRITE
extern	int	write	(int fd, const char *buffer, int size);
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif /* VILE_PROTO_H */
