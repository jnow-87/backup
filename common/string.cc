#include <string.h>


char *stralloc(char *_s, unsigned int len){
	char *s;

	s = new char[len + 1];

	if(s){
		if(_s)	strncpy(s, _s, len);
		else	len = 0;

		s[len] = 0;
	}

	return s;
}
