#ifndef STRING_H
#define STRING_H

/* macros */
#define DIRALLOC(str, len)({ \
	char *s; \
	\
	\
	if(len == 0 || ((str)[(len) - 1]) != '/'){ \
		s = stralloc(str, (len) + 1); \
		strcpy(s + len, (char*)"/"); \
	} \
	else  \
		s = stralloc(str, len); \
	\
	s; \
})

#define STRNULL(s)	((s) ? (s) : "")


/* prototypes */
char *stralloc(char *s, unsigned int len);


#endif // STRING_H
