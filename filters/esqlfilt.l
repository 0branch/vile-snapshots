%x COMMENT CODE SQL

%{
/*
 * $Header: /users/source/archives/vile.vcs/filters/RCS/esqlfilt.l,v 1.4 1999/12/27 00:23:00 tom Exp $
 *
 * Filter to add vile "attribution" sequences to embedded SQL with C/C++.
 * - T.Dickey
 */

#include <filters.h>

char *filter_name = "esql";

static char *other_symbols = "cpp";	/* FIXME: assumes ".merge cpp" */

static char *Action_attr;
static char *Comment_attr;
static char *Ident2_attr;
static char *Number_attr;
static char *Preproc_attr;
static char *String_attr;

static int my_state;
static int exec_sql;
static int in_execute;
static int in_block;

static void
default_state(int value)
{
    my_state = value;
    BEGIN(my_state);
}

static void
begin_sql(void)
{
    exec_sql = 2;
    set_symbol_table("esql");
    default_state(SQL);
}

static void
reset_symbol_table(void)
{
    exec_sql = 0;
    if (!in_execute && !in_block && my_state == SQL) {
	set_symbol_table(other_symbols);
	default_state(CODE);
    }
}

static void
process_keyword(char *text, int len)
{
    char *temp = "";
    char *attr = 0;

    switch (exec_sql) {
    case 0:
	/* Note: this will not highlight the "exec" keyword, because we do
	 * not have a reliable way to do lookahead without a grammar on top.
	 * However, most developers seem to be unaware that embedded SQL is
	 * case-independent, and will use either the upper or lowercase
	 * patterns below, with no intervening comments or blank lines.
	 */
	if (len == 4
	 && !strcmp(lowercase_of(text), "exec")) {
	    exec_sql = 1;
	}
	break;
    case 1:
	if (len == 3
	 && !strcmp(lowercase_of(text), "sql")) {
	    begin_sql();
	} else {
	    reset_symbol_table();
	}
	break;
    case 2:
	temp = lowercase_of(text);
	if (!strcmp(temp, "execute")) {
	    in_execute = 1;
	} else if (!strcmp(temp, "begin")) {
	    in_block = 1;
	} else if (!strcmp(temp, "end")) {
	    in_block = 0;
	}
	exec_sql++;
	break;
    }
    if ((exec_sql >= 2) || in_execute || in_block) {
	if (*temp == 0)
	    temp = lowercase_of(text);
	if ((attr = ci_keyword_attr(text)) == 0) {
	    set_symbol_table(other_symbols);
	    attr = keyword_attr(text);
	    set_symbol_table(filter_name);
	}
	if (in_execute
	 && (exec_sql == 0)
	 && !strcmp(temp, "end-exec")) {
	    in_execute = 0;
	    reset_symbol_table();
	}
    } else {
	attr = keyword_attr(text);
    }
    write_token(yyout, text, len, attr);
}

void
init_filter(int before GCC_UNUSED)
{
}

void
do_filter(FILE *input, FILE *output)
{
    yyin = input;
    yyout = output;

    Action_attr  = class_attr(NAME_ACTION);
    Comment_attr = class_attr(NAME_COMMENT);
    Ident2_attr  = class_attr(NAME_IDENT2);
    Number_attr  = class_attr(NAME_NUMBER);
    Preproc_attr = class_attr(NAME_PREPROC);
    String_attr  = class_attr(NAME_LITERAL);

    default_state(CODE);
    while (yylex() > 0) {
    }
}
%}

SPC		[ \t]
BLANK		{SPC}*

SSTRING		\'(\\.|[^'\\])*\'
DSTRING		\"(\\.|[^"\\])*\"
STRINGS		({SSTRING}|{DSTRING})

INCLUDE		(<[^>]+>|\"[^"]+\")

IDENT		[a-zA-Z_][a-zA-Z_0-9]*

SIGN		[-+]
DECIMAL		[0-9_]+
OCTAL		0[0-7_]+
HEXADECIMAL	0x[0-9a-fA-F_]+
REAL		[-+]?([0-9_]*\.[0-9][0-9_]*)([eE][+-]?[0-9_]+)?
NUMBER		{SIGN}?({DECIMAL}|{OCTAL}|{HEXADECIMAL}|{REAL})

%%

<CODE,SQL>^{BLANK}#include{BLANK}{INCLUDE} |
<CODE,SQL>^{BLANK}#{BLANK}{IDENT} { WriteToken(Preproc_attr); }

<CODE>"EXEC"{SPC}+"SQL"	|
<CODE>"exec"{SPC}+"sql"	{ WriteToken(Action_attr); begin_sql(); }

<SQL>{IDENT}-{IDENT}	|
<CODE,SQL>{IDENT}	{ process_keyword(yytext, yyleng); }
<CODE,SQL>";"		{ ECHO; reset_symbol_table(); }

<CODE,SQL>"/*"		{ WriteToken(Comment_attr); BEGIN(COMMENT); }
<COMMENT>[^*]*		{ WriteString(Comment_attr); }
<COMMENT>"*"+[^*/]*	{ WriteString(Comment_attr); }
<COMMENT>"*"+"/"	{ WriteString(Comment_attr); BEGIN(my_state); }

<CODE,SQL>"//"[^\n]*	{ WriteToken(Comment_attr); }

<CODE,SQL>{STRINGS}	{ WriteString(String_attr); }

<CODE,SQL>{NUMBER}	{ WriteString(Number_attr); }

<SQL>":"{IDENT}		{ WriteToken(Ident2_attr); }