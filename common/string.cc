#include <string.h>


char *stralloc(char const *_s, unsigned int len){
	char *s;

	s = new char[len + 1];

	if(s){
		if(_s)	strncpy(s, _s, len);
		else	len = 0;

		s[len] = 0;
	}

	return s;
}

char *stralloc(char const *s){
	return stralloc(s, strlen(s));
}

char *diralloc(char const *name, unsigned int len){
	char *s;


	if(len != 0 && name != 0 && name[len - 1] != '/'){
		s = stralloc(name, len + 1);
		s[len] = '/';
		s[len + 1] = 0;
	}
	else
		s = stralloc(name, len);

	return s;
}

char *diralloc(char const *name){
	return diralloc(name, strlen(name));
}

char *dirname(char const *base, char const *path){
	char *s;
	size_t i;


	if(path == 0 || (path[0] != '/' && base == 0))
		return 0;

	if(path[0] == '/')	i = strlen(path);
	else				i = strlen(base) + strlen(path);

	s = new char[i + 1];

	if(s == 0)
		return 0;

	/* concat strings */
	if(path[0] != '/'){
		strcpy(s, base);
		strcpy(s + strlen(base), path);
	}
	else
		strcpy(s, path);

	/* identify the directory part */
	// remove trailing '/'
	while(i > 0 && s[i - 1] == '/') --i;
	while(i > 0 && s[i - 1] != '/') --i;

	s[i] = 0;

	return s;
}
