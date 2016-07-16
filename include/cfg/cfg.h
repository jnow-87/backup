#ifndef CFG_H
#define CFG_H


/* macros */
#define CFG_PRINT(cfg, print){ \
	print("config: %s\n" \
		  "\tlogfile: %s\n" \
		  "\toutdir: %s\n" \
		  "\ttmpdir: %s\n" \
		  "\trsyncdir: %s\n" \
		  "\tbackup: %d\n" \
		  "\tpreserve: %d\n" \
		  "\tindicate: %d\n" \
		  "\tverbosity: %u\n" \
		  "\n", \
		  cfg->name, \
		  cfg->log_file, \
		  cfg->out_dir, \
		  cfg->tmp_dir, \
		  cfg->rsync_dir, \
		  cfg->backup, \
		  cfg->preserve, \
		  cfg->indicate, \
		  cfg->verbosity \
	); \
}


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

	bool backup,
		 indicate,
		 preserve;

	unsigned int verbosity;

	cfg_t *prev,
		  *next;
};


#endif // CFG_H
