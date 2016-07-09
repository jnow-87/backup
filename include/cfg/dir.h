#ifndef DIR_H
#define DIR_H


#include <cfg/file.h>


/* class */
class dir_t{
public:
	dir_t();
	~dir_t();

	char *path;
	file_t *file_lst;

	dir_t *prev,
		  *next;
};


#endif // DIR_H
