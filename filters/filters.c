/*
 * Common utility functions for vile syntax/highlighter programs
 *
 * $Header: /users/source/archives/vile.vcs/filters/RCS/filters.c,v 1.18 1998/12/22 02:46:56 tom Exp $
 *
 */

#include <filters.h>

#define MY_NAME "vile"

#ifdef DEBUG
#define TRACE(p) printf p;
#else
#define TRACE(p) /*nothing*/
#endif

#if HAVE_LONG_FILE_NAMES
#define KEYFILE_SUFFIX ".keywords"
#else
#define KEYFILE_SUFFIX ".key"
#endif

#if DOT_HIDES_FILE
#define DOT_TO_HIDE_IT "."
#else
#define DOT_TO_HIDE_IT ""
#endif

#define HASH_LENGTH 256

typedef struct _keyword KEYWORD;

struct _keyword {
    char *kw_name;
    char *kw_attr;
    unsigned kw_size;	/* strlen(kw_name) */
    KEYWORD *next;
};

static KEYWORD *hashtable[HASH_LENGTH];

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

static KEYWORD *
FindIdentifier(char *name)
{
	unsigned size = strlen(name);
	KEYWORD *hash_id;
	int Index;

	Index = hash_function(name);
	hash_id = hashtable[Index];
	while (hash_id != NULL) {
		if (hash_id->kw_size == size
		 && strcmp(hash_id->kw_name, name) == 0) {
			break;
		}
		hash_id = hash_id->next;
	}
	return hash_id;
}

static void
Free (char *ptr)
{
	if (ptr != 0)
		free(ptr);
}

/*
 * Find the first occurrence of a file in the canonical search list:
 *	the current directory
 *	the home directory
 *	vile's subdirectory of the home directory
 *	the vile library-directory
 *
 * On Unix we look for file/directory names with a "." prefixed since that
 * hides them.
 */
static FILE *
OpenKeywords(char *filename)
{
#define OPEN_IT(p) if ((fp = fopen(p, "r")) != 0) { TRACE(("Opened %s\n", p)) return fp; }
#define FIND_IT(p) sprintf p; OPEN_IT(name)

    static char *name;
    static unsigned have;

    FILE *fp;
    char *path;
    unsigned need;
    char leaf[20];

    if (strchr(filename, PATHSEP) != 0) {
	OPEN_IT(filename);
    }

    if ((path = getenv("HOME")) == 0)
	path = "";

    need = strlen(path)
	 + strlen(filename)
	 + 20;

    name = do_alloc(name, need, &have);

#if DOT_HIDES_FILE
    FIND_IT((name, "%s%c.%s", PATHDOT, PATHSEP, filename))
    FIND_IT((name, "%s%c.%s", path, PATHSEP, filename))
    sprintf(leaf, ".%s%c", MY_NAME, PATHSEP);
#else
    FIND_IT((name, "%s%c%s", PATHDOT, PATHSEP, filename))
    FIND_IT((name, "%s%c%s", path, PATHSEP, filename))
    sprintf(leaf, "%s%c", MY_NAME, PATHSEP);
#endif

    FIND_IT((name, "%s%c%s%s", path, PATHSEP, leaf, filename))

    path = getenv("VILE_STARTUP_PATH");
#ifdef VILE_STARTUP_PATH
    if (path == 0)
	path = VILE_STARTUP_PATH;
#endif
    if (path != 0) {
	int n = 0, m;
	name = do_alloc(name, strlen(path), &have);
	while (path[n] != 0) {
	    for (m = n; path[m] != 0 && path[m] != PATHCHR; m++)
		;
	    FIND_IT((name, "%.*s%c%s", m-n, path+n, PATHSEP, filename))
	    if (path[m])
		n = m+1;
	    else
		n = m;
	    }
    }

    return 0;
}

static void
ReadKeywords(char *filename)
{
    FILE *kwfile;
    char *attrs = 0;
    char *line  = 0;
    char *names = 0;
    char *s;
    int  items;
    unsigned attr_len = 0;
    unsigned line_len = 0;
    unsigned name_len = 0;

    TRACE(("ReadKeywords(%s)\n", filename))
    if ((kwfile = OpenKeywords(filename)) != NULL) {
	while (readline(kwfile, &line, &line_len) != 0) {
	    unsigned len = strlen(line);

	    while (len != 0
		 && isspace(line[len-1]))
		line[--len] = 0;
		if (!len)
		    continue;

	    names = do_alloc(names, line_len, &name_len);
	    attrs = do_alloc(attrs, line_len, &attr_len);
	    items = sscanf(line,
			    "%[^:]:%[IURC0-9A-F]",
			    names,
			    attrs);
	    TRACE(("ReadKeywords: Items %i, name=%s, attr=%s\n",
		    items, names, attrs))
	    if (items == 2 && *names && *attrs)
		insert_keyword(names, attrs);
	    else if (items == 1 && *names) {
		items = sscanf(line,
				"%[^:]:%s",
				names,
				attrs);
		if (items == 2 && *attrs && (s = keyword_attr(attrs)) != 0) {
		    insert_keyword(names, s);
		} else if ((s = keyword_attr(NAME_KEYWORD)) != 0) {
		    insert_keyword(names, s);
		}
	    }
	}
	fclose(kwfile);
    }
    Free(line);
    Free(names);
    Free(attrs);
}

static void
RemoveList(KEYWORD *k)
{
    if (k != NULL) {
	if (k->next != NULL)
	    RemoveList(k->next);
	free((char *)k);
    }
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

void
closehash(void)
{
    int i;
    for (i = 0; i < HASH_LENGTH; i++) {
	RemoveList(hashtable[i]);
	hashtable[i] = NULL; /* For unseen future i do this */
    }
}

char *
do_alloc(char *ptr, unsigned need, unsigned *have)
{
    need += 2;	/* allow for trailing null */
    if (need > *have) {
	if (ptr != 0)
	    ptr = realloc(ptr, need);
	else
	    ptr = malloc(need);
	*have = need;
    }
    return ptr;
}

void
inithash(void)
{
    int i;
    for (i = 0; i< HASH_LENGTH; i++)
	hashtable[i] = NULL;
}

char *
keyword_attr(char *name)
{
    KEYWORD *hash_id;
    if ((hash_id = FindIdentifier(name)) != 0)
	return hash_id->kw_attr;
    return 0;
}

char *
readline(FILE *fp, char **ptr, unsigned *len)
{
    char *buf = *ptr;
    unsigned used = 0;

    if (buf == 0)
	buf = (char *)malloc(*len = BUFSIZ);
    while (!feof(fp)) {
	int ch = fgetc(fp);
	if (ch == EOF || feof(fp) || ferror(fp)) {
	    break;
	}
	if (used+2 >= *len) {
	    *len = 3 * (*len) / 2;
	    buf = realloc(buf, *len);
	}
	buf[used++] = ch;
	if (ch == '\n')
	    break;
    }
    buf[used] = '\0';
    return used ? (*ptr = buf) : 0;
}

long
hash_function(char *id)
{
    /*
     * Build more elaborate hashing scheme. If you want one.
     */
    if ((id[0] == 0) || (id[1] == 0))
       return((int) id[0]);

    return (((int)id[0] ^ ((int)id[1] << 3) ^ ((int)id[2] >> 1)) & 0xff);
}

void
insert_keyword(char *ident, char *attribute)
{
    KEYWORD *first;
    KEYWORD *nxt;
    int Index;

    if ((nxt = FindIdentifier(ident)) != 0) {
	Free(nxt->kw_attr);
	nxt->kw_attr = strmalloc(attribute);
    } else {
	nxt = first = NULL;
	Index = hash_function(ident);
	first = hashtable[Index];
	if ((nxt = (KEYWORD *)malloc(sizeof(struct _keyword))) != NULL) {
	    nxt->kw_name = strmalloc(ident);
	    nxt->kw_size = strlen(nxt->kw_name);
	    nxt->kw_attr = strmalloc(attribute);
	    nxt->next = first;
	    hashtable[Index] = nxt;
	} else {
	    TRACE(("insert_keyword: cannot allocate\n"))
	    return;
	}
    }
    TRACE(("insert_keyword: nxt %p, kw_name %s, kw_size %i, kw_attr %s, next %p\n",
	nxt,
	nxt->kw_name,
	nxt->kw_size,
	nxt->kw_attr,
	nxt->next))
}

/*
 * We've parsed a string that is a case-independent name.
 */
int
is_ci_keyword(char *text)
{
    static char *name;
    static unsigned used;
    unsigned n;

    name = do_alloc(name, strlen(text), &used);
    for (n = 0; text[n] != 0; n++) {
	if (isalpha(text[n]) && isupper(text[n]))
	    name[n] = tolower(text[n]);
	else
	    name[n] = text[n];
    }
    name[n] = 0;
    return is_keyword(name);
}

/*
 * Lookup keywords in a table to avoid having flex pick out partial names,
 * e.g., "eval" from "eval_string".
 */
int
is_keyword(char *name)
{
    char *attr = keyword_attr(name);
    return (attr != 0 && (*attr == 0 || !strcmp(name, NAME_KEYWORD)));
}

char *
strmalloc(const char *src)
{
    register char *ns = (char *)malloc(strlen(src)+1);
    if (ns != 0)
	(void)strcpy(ns,src);
    return ns;
}

/*
 * The string may contain newlines, but vile processes the length only until
 * the end of line.
 */
void
write_string(FILE *fp, char *string, int length, char *marker)
{
    while (length > 0) {
	int n;
	for (n = 0; n < length && string[n] != 0 && string[n] != '\n'; n++)
	    ;
	if (n == 0) {
	    if (string[n])
		fputc(string[n], fp);
	} else {
	    fprintf(fp, "%c%i%s:%.*s", CTL_A, n, marker,
			(string[n] == '\n') ? n+1 : n, string);
	}
	if (string[n])
	    n++;
	length -= n;
	string += n;
    }
}

/*
 * Use this when you do not expect to handle embedded newlines.
 */
void
write_token(FILE *fp, char *string, int length, char *marker)
{
    if (length > 0) {
	if (marker != 0 && *marker != 0)
		fprintf(fp, "%c%i%s:", CTL_A, length, marker);
	fprintf(fp, "%.*s", length, string);
    }
}

int
main(int argc, char **argv)
{
    int k_used = 0;
    int n;

#if OPT_LOCALE
    setlocale(LC_CTYPE, "");
#endif

    inithash();

    insert_keyword(NAME_ACTION,  ATTR_ACTION);
    insert_keyword(NAME_COMMENT, ATTR_COMMENT);
    insert_keyword(NAME_IDENT,   ATTR_IDENT);
    insert_keyword(NAME_IDENT2,  ATTR_IDENT2);
    insert_keyword(NAME_KEYWORD, ATTR_KEYWORD);
    insert_keyword(NAME_LITERAL, ATTR_LITERAL);
    insert_keyword(NAME_NUMBER,  ATTR_NUMBER);
    insert_keyword(NAME_PREPROC, ATTR_PREPROC);
    insert_keyword(NAME_TYPES,   ATTR_TYPES);

    init_filter();

    for (n = 1; n < argc; n++) {
	char *s = argv[n];
	if (*s == '-') {
	    while (*++s) {
		switch(*s) {
		case 'k':
		    ReadKeywords(argv[n]);
		    k_used++;
		    break;
		default:
		    fprintf(stderr, "unknown option %c\n", *s);
		    exit(1);
		}
	    }
	} else {
	    break;
	}
    }
    if (!k_used) {
	char name[80];

	sprintf(name, "%s%s", MY_NAME, KEYFILE_SUFFIX);
	ReadKeywords(name);

	if (strcmp(MY_NAME, filter_name)) {
	    sprintf(name, "%s%s", filter_name, KEYFILE_SUFFIX);
	    ReadKeywords(name);
	}
    }
    if (n < argc) {
	char *name = argv[n++];
	FILE *fp = fopen(name, "r");
	if (fp != 0) {
	    do_filter(fp, stdout);
	    fclose(fp);
	}
    } else {
	do_filter(stdin, stdout);
    }
    closehash();

    exit(0);
}
