%s CODE NOTE EXP1 EXP2

%{
/*
 * $Header: /users/source/archives/vile.vcs/filters/RCS/est-filt.l,v 1.2 2002/10/11 16:44:02 tom Exp $
 *
 * Filter to add vile "attribution" sequences to selected bits of enscript
 * syntax-description files.
 */

#include <filters.h>

DefineFilter("est");

static char *Comment_attr;
static char *Ident_attr;
static char *Keyword_attr;
static char *Number_attr;
static char *String_attr;

static void
init_filter(int before GCC_UNUSED)
{
}

static void
do_filter(FILE *input)
{
    yyin = input;

    Comment_attr = class_attr(NAME_COMMENT);
    Ident_attr   = class_attr(NAME_IDENT);
    Keyword_attr = class_attr(NAME_KEYWORD);
    Number_attr  = class_attr(NAME_NUMBER);
    String_attr  = class_attr(NAME_LITERAL);

    BEGIN(CODE);

    while (yylex() > 0) {
    }
    flt_bfr_error();
}

%}

SSTRING		\'(\\.|[^'\\])*\'
DSTRING		\"((\\\")|[^"\n])*\"

KEYWORD		[a-zA-Z_][a-zA-Z_0-9]*
IDENT		\$[a-zA-Z_0-9]+

SIGN		[-+]
INTEGER		{SIGN}?([0-9]+)
REAL		{SIGN}?[0-9]+\.[0-9]*|{SIGN}?\.[0-9]+

RANGE		"["(\\.|.)+"]"

%%

<CODE>"/*"			{ BeginQuote(NOTE, Comment_attr); }
<NOTE>"*"[^/]			{ flt_bfr_append(yytext, yyleng); }
<NOTE>\n			{ flt_bfr_append(yytext, yyleng); }
<NOTE>[^*\n]			{ flt_bfr_append(yytext, yyleng); }
<NOTE>"*/"			{ FinishQuote(CODE); }

<CODE>{KEYWORD}			{ WriteToken(keyword_attr(yytext)); }

<CODE>{INTEGER}	|
<CODE>{REAL}			{ WriteToken(Number_attr); }

<CODE>{IDENT}			{ WriteToken(Ident_attr); }

<CODE>{SSTRING}			{ WriteToken(String_attr); }
<CODE>{DSTRING}			{ WriteToken(String_attr); }

<CODE>"/"			{ BeginQuote(EXP1, String_attr); }
<EXP1>\\.			{ flt_bfr_append(yytext, yyleng); BEGIN(EXP2); }
<EXP1>{RANGE}			{ flt_bfr_append(yytext, yyleng); BEGIN(EXP2); }
<EXP1>[^*\\]			{ flt_bfr_append(yytext, yyleng); BEGIN(EXP2); }
<EXP2>\\.			{ flt_bfr_append(yytext, yyleng); }
<EXP2>{RANGE}			{ flt_bfr_append(yytext, yyleng); }
<EXP2>[^/]			{ flt_bfr_append(yytext, yyleng); }
<EXP1,EXP2>"/"			{ FinishQuote(CODE); }