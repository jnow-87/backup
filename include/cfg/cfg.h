#ifndef CFG_H
#define CFG_H


#include <cfg/script.h>


/* macros */
#define CFG_PRINT(cfg, print){ \
	print("\t%s\n" \
		  "\t\tlogfile: %s\n" \
		  "\t\toutdir: %s\n" \
		  "\t\trsyncdir: %s\n" \
		  "\t\ttmpdir: %s\n" \
		  "\t\tarchive: %d\n" \
		  "\t\tpreserve: %d\n" \
		  "\t\tindicate: %d\n" \
		  "\t\tnoconfig: %d\n" \
		  "\t\tnodata: %d\n" \
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
		  cfg->noconfig, \
		  cfg->nodata, \
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
		 preserve,
		 noconfig,
		 nodata;

	unsigned int verbosity;

	script_t *pre_backup_lst,
			 *post_backup_lst,
			 *pre_restore_lst,
			 *post_restore_lst;

	cfg_t *prev,
		  *next;
};


#endif // CFG_H
