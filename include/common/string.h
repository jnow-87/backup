#ifndef STRING_H
#define STRING_H

/* macros */
#define MAXLEN	512

#define XCHG(a, b){ \
	auto c = a; \
	a = b; \
	b = c; \
}

#define STRNULL(s)	((s) ? (s) : "")

#define PATHCONCAT(base, dir) \
	(((dir) != 0 && (dir)[0] == '/') ? "" : STRNULL(base)), STRNULL(dir)


/* types */
typedef enum{
	PF_DIR = 0x1,
	PF_FILE = 0x2,
	PF_ABS = 0x4,
	PF_REL = 0x8
} path_flags_t;


/* prototypes */
char *stralloc(char const *name, unsigned int len, int flags);
char *stralloc(char const *name, int flags);
char *dirname(char const *base, char const *path);


#endif // STRING_H
