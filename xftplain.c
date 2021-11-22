/*
 * Xft text-output, Thomas Dickey 2020
 *
 * $Id: xftplain.c,v 1.48 2021/11/21 23:34:23 tom Exp $
 *
 * Some of this was adapted from xterm, of course.
 */

#include <x11vile.h>

#define GetFcBool(pattern, what) \
    (FcPatternGetBool(pattern, what, 0, &fc_bool) == FcResultMatch)

#define GetFcInt(pattern, what) \
    (FcPatternGetInteger(pattern, what, 0, &fc_int) == FcResultMatch)

#define GetFcTxt(pattern, what) \
    (FcPatternGetString(pattern, what, 0, &fc_txt) == FcResultMatch)

#define NormXftPattern \
	    XFT_FAMILY,     XftTypeString, "mono", \
	    FC_COLOR,       XftTypeBool,   FcFalse, \
	    FC_OUTLINE,     XftTypeBool,   FcTrue, \
	    XFT_SIZE,       XftTypeDouble, face_size

#define BoldXftPattern(font) \
	    XFT_WEIGHT,     XftTypeInteger, XFT_WEIGHT_BOLD, \
	    XFT_CHAR_WIDTH, XftTypeInteger, (font)->max_advance_width

#define ItalXftPattern(font) \
	    XFT_SLANT,      XftTypeInteger, XFT_SLANT_ITALIC, \
	    XFT_CHAR_WIDTH, XftTypeInteger, (font)->max_advance_width

#define BtalXftPattern(font) \
	    XFT_WEIGHT,     XftTypeInteger, XFT_WEIGHT_BOLD, \
	    XFT_SLANT,      XftTypeInteger, XFT_SLANT_ITALIC, \
	    XFT_CHAR_WIDTH, XftTypeInteger, (font)->max_advance_width

#define MAX_FONTNAME 1024

static double face_size = 12.0;	/* FIXME - make this configurable */

static XVileFont *
open_font(Display *dpy, XftPattern *temper, char *fullname)
{
    XftPattern *match;
    XftResult status;
    XVileFont *result = NULL;

    if (fullname != NULL)
	*fullname = '\0';

    FcConfigSubstitute(NULL, temper, FcMatchPattern);
    XftDefaultSubstitute(dpy, DefaultScreen(dpy), temper);

    match = FcFontMatch(NULL, temper, &status);
    if (match != NULL) {
	int fc_int = -1;
	result = XftFontOpenPattern(dpy, match);

	if (GetFcInt(temper, FC_SPACING) && fc_int != FC_MONO) {
	    fprintf(stderr, "fc_int:%d\n", fc_int);
	    (void) fprintf(stderr,
			   "proportional font, things will be miserable\n");
	}
	/*
	 * Xft is a layer over fontconfig.  The latter's documentation
	 * is composed of circular definitions which never get around
	 * to defining the format returned by FcNameUnparse.  The value
	 * returned via Xft has (also undocumented) the fontname that
	 * we want first in the buffer.
	 */
	if (fullname != NULL && XftNameUnparse(match, fullname, MAX_FONTNAME)) {
	    char *colon = strchr(fullname, ':');
	    if (colon != NULL)
		*colon = '\0';
	}
    }
    return result;
}

static XVileFont *
open_font_pattern(Display *dpy, char *fullname, ...)
{
    XftPattern *pat;
    XVileFont *result = NULL;

    if ((pat = XftNameParse(fullname)) != 0) {
	XftPattern *temper;

	if ((temper = XftPatternDuplicate(pat)) != NULL) {
	    va_list ap;
	    va_start(ap, fullname);
	    XftPatternVaBuild(temper, ap);
	    va_end(ap);

	    result = open_font(dpy, temper, fullname);

	    /* FIXME xterm retains the pattern to (try to) make the
	     * bold/italic versions follow the same pattern as normal -
	     * should xvile?
	     */
	    XftPatternDestroy(temper);
	}
    }
    return result;
}

static XVileFont *
alternate_font(Display *dpy, TextWindow win, int bold, int slant)
{
    XVileFont *result = NULL;
    char fullname[MAX_FONTNAME];
    char *fn = strcpy(fullname, x_current_fontname());
    (void) dpy;

    if (bold) {
	if (slant) {
	    if ((result = win->pfont_boldital) == NULL) {
		result = open_font_pattern(dpy, fn,
					   NormXftPattern,
					   BtalXftPattern(win->pfont),
					   NULL);
	    }
	} else {
	    if ((result = win->pfont_bold) == NULL) {
		result = open_font_pattern(dpy, fn,
					   NormXftPattern,
					   BoldXftPattern(win->pfont),
					   NULL);
	    }
	}
    } else {
	if (slant) {
	    if ((result = win->pfont_ital) == NULL) {
		result = open_font_pattern(dpy, fn,
					   NormXftPattern,
					   ItalXftPattern(win->pfont),
					   NULL);
	    }
	} else {
	    if ((result = win->pfont) == NULL) {
		result = open_font_pattern(dpy, fn, NormXftPattern, NULL);
	    }
	}
    }
    return result;
}

static void
really_draw(Display *dpy,
	    TextWindow win,
	    ColorGC * fore_ptr,
	    VIDEO_TEXT * text,
	    int tlen,
	    unsigned attr,
	    int sr,
	    int sc)
{
#define MYSIZE 1024
    XftColor color = ((attr & VAREV)
		      ? fore_ptr->xft.reverse
		      : fore_ptr->xft.normal);
    int y = text_y_pos(win, sr);
    int x = x_pos(win, sc);
    XftCharSpec buffer[MYSIZE];
    static XftDraw *renderDraw;

    TRACE(("really_draw [%d,%d] %s\n", sr, sc, visible_video_text(text, tlen)));
    if (renderDraw == 0) {
	int scr = DefaultScreen(dpy);
	Visual *visual = DefaultVisual(dpy, scr);
	renderDraw = XftDrawCreate(dpy,
				   win->win, visual,
				   DefaultColormap(dpy, scr));
    }
    while (tlen > 0) {
	Cardinal n;

	for (n = 0; ((int) n < tlen) && (n < MYSIZE); ++n) {
	    buffer[n].ucs4 = text[n];
	    buffer[n].x = (short) (x + win->char_width * n);
	    buffer[n].y = (short) (y);
	}

	XftDrawCharSpec(renderDraw,
			&color,
			win->curfont,
			buffer,
			(int) n);

	tlen -= MYSIZE;
	if (tlen > 0) {
	    text += MYSIZE;
	    sc += MYSIZE;
	}
    }
#undef MYSIZE
}

/*
 * The X protocol request for clearing a rectangle (PolyFillRectangle) takes
 * 20 bytes.  It will therefore be more expensive to switch from drawing text
 * to filling a rectangle unless the area to be cleared is bigger than 20
 * spaces.  Actually it is worse than this if we are going to switch
 * immediately to drawing text again since we incur a certain overhead
 * (16 bytes) for each string to be displayed.  This is how the value of
 * CLEAR_THRESH was computed (36 = 20+16).
 *
 * Kev's opinion:  If XDrawImageString is to be called, it is hardly ever
 * worth it to call XFillRectangle.  The only time where it will be a big
 * win is when the entire area to update is all spaces (in which case
 * XDrawImageString will not be called).  The following code would be much
 * cleaner, simpler, and easier to maintain if we were to just call
 * XDrawImageString where there are non-spaces to be written and
 * XFillRectangle when the entire region is to be cleared.
 */
#define	CLEAR_THRESH	36

void
xvileDraw(Display *dpy,
	  TextWindow win,
	  VIDEO_TEXT * text,
	  int len,
	  UINT attr,
	  int sr,
	  int sc)
{
    ColorGC *fore_ptr;
    ColorGC *back_ptr;
    int fore_yy = text_y_pos(win, sr);
    int back_yy = y_pos(win, sr);
    VIDEO_TEXT *p;
    int cc, tlen, i, startcol;
    int fontchanged = FALSE;

    if (win->curfont == NULL)
	win->curfont = win->pfont;

    if (attr == 0) {		/* This is the most common case, so we list it first */
	fore_ptr = &(win->tt_info);
	back_ptr = &(win->rt_info);
    } else if ((attr & VACURS) && win->is_color_cursor) {
	fore_ptr = &(win->cc_info);
	back_ptr = &(win->rc_info);
	attr &= ~VACURS;
    } else if (attr & VASEL) {
	fore_ptr = &(win->ss_info);
	back_ptr = &(win->rs_info);
    } else if (attr & VAMLFOC) {
	fore_ptr = &(win->mm_info);
	back_ptr = &(win->mm_info);
    } else if (attr & VAML) {
	fore_ptr = &(win->rm_info);
	back_ptr = &(win->rm_info);
    } else if (attr & (VACOLOR)) {
	int fg = ctrans[VCOLORNUM(attr)];
	int bg = (gbcolor == ENUM_FCOLOR) ? fg : ctrans[gbcolor];

	if (attr & (VAREV)) {
	    fore_ptr = x_get_color_gc(win, fg, False);
	    back_ptr = x_get_color_gc(win, bg, True);
	    attr &= ~(VAREV);
	} else {
	    fore_ptr = x_get_color_gc(win, fg, True);
	    back_ptr = x_get_color_gc(win, bg, False);
	}
    } else {
	fore_ptr = &(win->tt_info);
	back_ptr = &(win->rt_info);
    }

    if (attr & (VAREV | VACURS)) {
	ColorGC *temp_ptr = fore_ptr;
	fore_ptr = back_ptr;
	back_ptr = temp_ptr;
    }

    if (attr & (VABOLD | VAITAL)) {
	XVileFont *fsp = NULL;
	if ((attr & (VABOLD | VAITAL)) == (VABOLD | VAITAL)) {
	    if (!(win->fsrch_flags & FSRCH_BOLDITAL)) {
		if ((fsp = alternate_font(dpy, win, TRUE, TRUE)) != NULL
		    || (fsp = alternate_font(dpy, win, TRUE, FALSE)) != NULL)
		    win->pfont_boldital = fsp;
		win->fsrch_flags |= FSRCH_BOLDITAL;
	    }
	    if (win->pfont_boldital != NULL) {
		win->curfont = win->pfont_boldital;
		fontchanged = TRUE;
		attr &= ~(VABOLD | VAITAL);	/* don't use fallback */
	    } else
		goto tryital;
	} else if (attr & VAITAL) {
	  tryital:
	    if (!(win->fsrch_flags & FSRCH_ITAL)) {
		if ((fsp = alternate_font(dpy, win, FALSE, TRUE)) != NULL
		    || (fsp = alternate_font(dpy, win, FALSE, FALSE))
		    != NULL)
		    win->pfont_ital = fsp;
		win->fsrch_flags |= FSRCH_ITAL;
	    }
	    if (win->pfont_ital != NULL) {
		win->curfont = win->pfont_ital;
		fontchanged = TRUE;
		attr &= ~VAITAL;	/* don't use fallback */
	    } else if (attr & VABOLD)
		goto trybold;
	} else if (attr & VABOLD) {
	  trybold:
	    if (!(win->fsrch_flags & FSRCH_BOLD)) {
		win->pfont_bold = alternate_font(dpy, win, TRUE, FALSE);
		win->fsrch_flags |= FSRCH_BOLD;
	    }
	    if (win->pfont_bold != NULL) {
		win->curfont = win->pfont_bold;
		fontchanged = TRUE;
		attr &= ~VABOLD;	/* don't use fallback */
	    }
	}
    }

    XFillRectangle(dpy, win->win, back_ptr->gc,
		   x_pos(win, sc), back_yy,
		   (UINT) (len * win->char_width),
		   (UINT) (win->char_height));

    /* break line into TextStrings and FillRects */
    p = text;
    cc = 0;
    tlen = 0;
    startcol = sc;
    for (i = 0; i < len; i++) {
	if (text[i] == ' ') {
	    cc++;
	    tlen++;
	} else {
	    if (cc >= CLEAR_THRESH) {
		tlen -= cc;
		really_draw(dpy, win, fore_ptr, p, tlen, attr, sr, sc);
		p += tlen + cc;
		sc += tlen;
		XFillRectangle(dpy, win->win, back_ptr->gc,
			       x_pos(win, sc), back_yy,
			       (UINT) (cc * win->char_width),
			       (UINT) (win->char_height));
		sc += cc;
		tlen = 1;	/* starting new run */
	    } else
		tlen++;
	    cc = 0;
	}
    }
    if (cc >= CLEAR_THRESH) {
	tlen -= cc;
	really_draw(dpy, win, fore_ptr, p, tlen, attr, sr, sc);
	sc += tlen;
	XFillRectangle(dpy, win->win, back_ptr->gc,
		       x_pos(win, sc), back_yy,
		       (UINT) (cc * win->char_width),
		       (UINT) (win->char_height));
    } else if (tlen > 0) {
	really_draw(dpy, win, fore_ptr, p, tlen, attr, sr, sc);
    }
    if (attr & (VAUL | VAITAL)) {
	fore_yy += win->char_descent - 1;
	XDrawLine(dpy, win->win, fore_ptr->gc,
		  x_pos(win, startcol), fore_yy,
		  x_pos(win, startcol + len) - 1, fore_yy);
    }
    if (fontchanged) {
	win->curfont = win->pfont;
    }
}

/*
 * Given the Xft font metrics, determine the actual font size.  This is used
 * for each font to ensure that normal, bold and italic fonts follow the same
 * rule.
 */
static void
setRenderFontsize(TextWindow tw, XftFont *font, const char *tag)
{
    if (font != 0) {
	int width, height, ascent, descent;
#ifdef OPT_TRACE
	FT_Face face;
	FT_Size size;
	FT_Size_Metrics metrics;
	Boolean scalable;
	Boolean is_fixed;
	Boolean debug_xft = False;

	face = XftLockFace(font);
	size = face->size;
	metrics = size->metrics;
	is_fixed = FT_IS_FIXED_WIDTH(face);
	scalable = FT_IS_SCALABLE(face);
	XftUnlockFace(font);

	/* freetype's inconsistent for this sign */
	metrics.descender = -metrics.descender;

#define TR_XFT	   "Xft metrics: "
#define D_64(name) ((double)(metrics.name)/64.0)
#define M_64(a,b)  ((font->a * 64) != metrics.b)
#define BOTH(a,b)  D_64(b), M_64(a,b) ? "*" : ""

	debug_xft = (M_64(ascent, ascender)
		     || M_64(descent, descender)
		     || M_64(height, height)
		     || M_64(max_advance_width, max_advance));

	TRACE(("Xft font is %sscalable, %sfixed-width\n",
	       is_fixed ? "" : "not ",
	       scalable ? "" : "not "));

	if (debug_xft) {
	    TRACE(("Xft font size %d+%d vs %d by %d\n",
		   font->ascent,
		   font->descent,
		   font->height,
		   font->max_advance_width));
	    TRACE((TR_XFT "ascender    %6.2f%s\n", BOTH(ascent, ascender)));
	    TRACE((TR_XFT "descender   %6.2f%s\n", BOTH(descent, descender)));
	    TRACE((TR_XFT "height      %6.2f%s\n", BOTH(height, height)));
	    TRACE((TR_XFT "max_advance %6.2f%s\n", BOTH(max_advance_width, max_advance)));
	} else {
	    TRACE((TR_XFT "matches font\n"));
	}
#endif

	width = font->max_advance_width;
	height = font->height;
	ascent = font->ascent;
	descent = font->descent;

	/*
	 * An empty tag is used for the base/normal font.
	 */
	if (isEmpty(tag)) {
	    tw->char_width = width;
	    tw->char_height = height;
	    tw->char_ascent = ascent;
	    tw->char_descent = descent;
	    tw->left_ink = 0;
	    tw->right_ink = 0;
	}
    }
}

/*
 * from xterm, in turn adapted from xfd/grid.c
 */
static FcChar32
xft_last_char(XftFont *xft)
{
    FcChar32 this, last, next;
    FcChar32 map[FC_CHARSET_MAP_SIZE];
    int i;
    last = FcCharSetFirstPage(xft->charset, map, &next);
    while ((this = FcCharSetNextPage(xft->charset, map, &next)) != FC_CHARSET_DONE)
	last = this;
    last &= (FcChar32) ~ 0xff;
    for (i = FC_CHARSET_MAP_SIZE - 1; i >= 0; i--) {
	if (map[i]) {
	    FcChar32 bits = map[i];
	    last += (FcChar32) i *32 + 31;
	    while (!(bits & 0x80000000)) {
		last--;
		bits <<= 1;
	    }
	    break;
	}
    }
    return (FcChar32) last;
}

XVileFont *
xvileQueryFont(Display *dpy, TextWindow tw, const char *fname)
{
    XVileFont *pf;
    char fullname[MAX_FONTNAME + 2];

    TRACE(("x11:query_font(%s)\n", fname));
    pf = NULL;
    sprintf(fullname, "%.*s", MAX_FONTNAME, fname);
    pf = open_font_pattern(dpy, fullname, NormXftPattern, NULL);
    if (pf != NULL) {
	/*
	 * Free resources associated with any presently loaded fonts.
	 */
	if (tw->pfont)
	    XftFontClose(dpy, tw->pfont);
	if (tw->pfont_bold) {
	    XftFontClose(dpy, tw->pfont_bold);
	    tw->pfont_bold = NULL;
	}
	if (tw->pfont_ital) {
	    XftFontClose(dpy, tw->pfont_ital);
	    tw->pfont_ital = NULL;
	}
	if (tw->pfont_boldital) {
	    XftFontClose(dpy, tw->pfont_boldital);
	    tw->pfont_boldital = NULL;
	}
	tw->fsrch_flags = 0;

	tw->pfont = pf;

	setRenderFontsize(tw, pf, NULL);

	x_set_fontname(tw, fullname);
#if OPT_MULTIBYTE
	x_set_font_encoding(xft_last_char(pf) > 255 ? enc_UTF8 : enc_8BIT);
#endif
    }
    return pf;
}
