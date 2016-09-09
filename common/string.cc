#include <string.h>
#include <common/string.h>


char *stralloc(char const *name, unsigned int len, int flags){
	char *s,
		 *r,
		 cfirst,
		 clast;
	int ext;


	ext = 0;
	cfirst = name[0];
	clast = name[(len > 0 ? len - 1 : 0)];

	/* check flags */
	// add char for trailing '/'
	if((flags & PF_DIR) && clast != '/')	++ext;

	// do not copy trailing '/'
	if((flags & PF_FILE) && clast == '/')	--len;

	// add leading '/'
	if((flags & PF_ABS) && cfirst != '/')	++ext;

	// do not copy leading '/'
	if((flags & PF_REL) && cfirst == '/'){
		--len;
		++name;
	}

	/* allocate */
	s = new char[len + ext + 1];
	r = s;

	if(s == 0)
		return 0;

	/* terminate string */
	s[len + ext] = 0;

	/* add leading '/' if name is not an absolute path that it is supposed to be */
	if((flags & PF_ABS) && cfirst != '/'){
		s[0] = '/';
		s++;
	}

	/* copy name */
	if(name)	strncpy(s, name, len);
	else		s[0] = 0;

	/* add trailing '/' if its missing */
	if((flags & PF_DIR) && clast != '/')
		s[len] = '/';

	return r;
}

char *stralloc(char const *name, int flags){
	return stralloc(name, strlen(name), flags);
}

char *dirname(char const *base, char const *dir, char const *file){
	char *s;
	size_t i;


	base = STRNULL(base);
	dir = STRNULL(dir);
	file = STRNULL(file);

	/* allocate */
	i = strlen(base) + strlen(dir) + strlen(file);;

	if(base[0] != 0 && dir[0] == '/'){
		--i;
		++dir;
	}

	s = new char[i + 1];

	if(s == 0)
		return 0;

	/* concat strings */
	strcpy(s, base);
	strcpy(s + strlen(base), dir);
	strcpy(s + i - strlen(file), file);

	/* identify the directory part */
	// remove trailing '/'
	while(i > 0 && s[i - 1] == '/') --i;
	while(i > 0 && s[i - 1] != '/') --i;

	s[i] = 0;

	return s;
}

char const *filename(char const *file){
	char const *p;


	if(*file == 0)
		return file;

	p = file + strlen(file) - 1;

	/* skip trailing '/' */
	while(p != file && *p == '/') --p;

	/* skip to the next '/' or start of string */
	while(p != file && *p != '/') --p;

	return *p == '/' ? p + 1 : p;
}
