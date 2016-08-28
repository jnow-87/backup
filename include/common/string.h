#ifndef STRING_H
#define STRING_H

/* macros */
#define MAXLEN	256

#define XCHG(a, b){ \
	auto c = a; \
	a = b; \
	b = c; \
}

#define STRNULL(s)	((s) ? (s) : "")


/* prototypes */
char *stralloc(char const *s, unsigned int len);
char *stralloc(char const *s);
char *diralloc(char const *name, unsigned int len);
char *diralloc(char const *name);
char *dirname(char const *base, char const *path);


#endif // STRING_H
