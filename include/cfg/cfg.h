#ifndef CFG_H
#define CFG_H


/* class */
class cfg_t{
public:
	cfg_t();
	~cfg_t();

	char *name;

	char *out_dir,
		 *tmp_dir,
		 *rsync_dir;

	char *log_file;

	bool archive,
		 indicate,
		 preserve,
		 verbose;

	cfg_t *prev,
		  *next;
};


#endif // CFG_H
