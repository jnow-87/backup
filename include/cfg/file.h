#ifndef FILE_H
#define FILE_H


/* class */
class file_t{
public:
	file_t();
	~file_t();

	char *name,
		 *rsync_dir;

	file_t *prev,
		   *next;
};


#endif // FILE_H
