%s RULES ACTIONS CODE COMMENT

%{
/*
 * $Header: /users/source/archives/vile.vcs/filters/RCS/yaccfilt.l,v 1.8 1998/12/28 21:52:10 tom Exp $
 *
 * Filter to add vile "attribution" sequences to selected bits of YACC program.
 */

#include <filters.h>

char *filter_name = "yacc";

static char *Comment_attr;
static char *Ident_attr;
static char *Ident2_attr;
static char *Keyword_attr;
static char *Number_attr;
static char *Preproc_attr;
static char *String_attr;

static char *save_attr = "";

#define MAX_LEVEL 10

static int oldstate = INITIAL;
static int section = 0;
static int state_level = 0;
static int brace_level = 0;

static int state_stack[MAX_LEVEL];

static void
new_state(int code)
{
    oldstate = code;
    BEGIN(code);
}

static void
push_state(int code)
{
    state_stack[state_level++] = oldstate;
    new_state(code);
}

static void
pop_state(void)
{
    if (state_level > 0)
	new_state(state_stack[--state_level]);
}

static void
set_state(void)
{
    if (section >= 2) {
	new_state(CODE);
    } else if (section >= 1) {
	new_state(ACTIONS);
    } else {
	new_state(RULES);
    }
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
    Comment_attr = keyword_attr(NAME_COMMENT);
    Ident_attr   = keyword_attr(NAME_IDENT);
    Ident2_attr  = keyword_attr(NAME_IDENT2);
    Keyword_attr = keyword_attr(NAME_KEYWORD);
    Number_attr  = keyword_attr(NAME_NUMBER);
    Preproc_attr = keyword_attr(NAME_PREPROC);
    String_attr  = keyword_attr(NAME_LITERAL);

    new_state(RULES);
    while (yylex() > 0) {
    }
}

%}

SPACE		[ \t]

DIRECTIVE	^%[^ \t\n]+

IDENT		[a-zA-Z_][a-zA-Z_0-9]*
QIDENT		\$[0-9$]

INTEGER		[-+]?([0-9]+)

SSTRING		\'((\\.)|[^\'])*\'
DSTRING		\"((\\.)|[^\"])*\"
STRINGS		({SSTRING}|{DSTRING})

PATTERN		(\\\.|[^ \t\n])+

%%

<INITIAL,RULES,ACTIONS,CODE>{DIRECTIVE}	{
		    save_attr = "";
		    WriteToken(Keyword_attr);
		    switch(yytext[1]) {
		    case '%':
		    	section++;
			set_state();
			break;
		    case '{':
		    	new_state(CODE);
			break;
		    case '}':
			set_state();
			break;
		    default:
			if (!strcmp(yytext+1, "token")) {
			    save_attr = Ident2_attr;
			} else if (!strcmp(yytext+1, "type")) {
			    save_attr = Ident_attr;
			}
			break;
		    }
		}

<RULES>{IDENT}		{ WriteToken(save_attr); insert_keyword(yytext, save_attr); }
<RULES>\<{IDENT}\>	{ WriteToken(Ident2_attr); }
<RULES>\{		{ ECHO; push_state(CODE); }
<RULES>;		{ WriteToken(String_attr); }

<ACTIONS>{IDENT}	{ WriteToken(keyword_attr(yytext)); }
<ACTIONS>\{		{ ECHO; brace_level = 1; push_state(CODE); }
<ACTIONS>;		{ WriteToken(String_attr); }

<CODE>{IDENT}		{ WriteToken(keyword_attr(yytext)); }
<CODE>{INTEGER}		{ WriteToken(Number_attr); }
<CODE>\{		{ ECHO; brace_level++; }
<CODE>\}		{ ECHO; if (--brace_level <= 0) { brace_level = 0; pop_state(); } }

"/*"			{ WriteToken(Comment_attr); push_state(COMMENT); }
<COMMENT>[^*]*		{ WriteString(Comment_attr); }
<COMMENT>"*"+[^*/]*	{ WriteString(Comment_attr); }
<COMMENT>"*"+"/"	{ WriteToken(Comment_attr); pop_state(); }

^{SPACE}*#{SPACE}*{IDENT}({SPACE}+(\<[^>]+\>|\"[^"]+\"))? { WriteString(Preproc_attr); }
{QIDENT}		{ WriteToken(Ident2_attr); }
{STRINGS}		{ WriteString(String_attr); }