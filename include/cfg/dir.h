#ifndef DIR_H
#define DIR_H


#include <cfg/file.h>


/* macros */
#define DIR_PRINT(dir, print){ \
	file_t *file; \
	\
	\
	print("\t%s\n", dir->path); \
	\
	list_for_each(dir->file_lst, file) \
		if(file->rsync_dir)	print("\t\tfile: %s (rsync-dir: %s)\n", file->name, file->rsync_dir); \
		else				print("\t\tfile: %s\n", file->name); \
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
