#ifndef DIR_H
#define DIR_H


#include <cfg/file.h>


/* macros */
#define DIR_PRINT(dir, print){ \
	file_t *file; \
	\
	\
	print("directory: %s\n", dir->path); \
	\
	list_for_each(dir->file_lst, file) \
		print("\tfile: %s %s\n", file->name, file->rsync_dir); \
	\
	print("\n"); \
}


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
