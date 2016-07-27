#ifndef CFG_H
#define CFG_H


/* macros */
#define CFG_PRINT(print, cfg){ \
	print("\t%s\n" \
		  "\t\tlogfile: %s\n" \
		  "\t\toutdir: %s\n" \
		  "\t\trsyncdir: %s\n" \
		  "\t\ttmpdir: %s\n" \
		  "\t\tarchive: %d\n" \
		  "\t\tpreserve: %d\n" \
		  "\t\tindicate: %d\n" \
		  "\t\tverbosity: %u\n" \
		  "\n", \
		  cfg->name, \
		  cfg->log_file ? cfg->log_file : "-", \
		  cfg->out_dir ? cfg->out_dir : "-", \
		  cfg->rsync_dir ? cfg->rsync_dir : "-", \
		  cfg->tmp_dir ? cfg->tmp_dir : "-", \
		  cfg->archive, \
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

	bool archive,
		 indicate,
		 preserve;

	unsigned int verbosity;

	cfg_t *prev,
		  *next;
};


#endif // CFG_H
