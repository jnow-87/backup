#include <common/log.h>
#include <common/argv.h>
#include <common/list.h>
#include <common/escape.h>
#include <cfg/parser.tab.h>
#include <stdio.h>


/**
 * TODO
 * 	- set tmp directory through command line and config
 * 	- verbose output via different levels
 * 	- main functionality
 * 		- rsync_dir is relative to out_dir or absolute
 * 		- options: handle_normale handle_rsync
 * 		- handle "special" directories like "homes"
 *
 *	- config variables resolved later
 */
int main(int argc, char **argv){
	bool valid;
	cfg_t *cfg_lst,
		  *cfg;
	dir_t *dir_lst,
		  *dir;
	file_t *file;


	if(argv::parse(argc, argv) != 0)
		return 1;

	log::set_log_level(argv::verbosity);

	ARGV_PRINT();

	cfgparse(argv::config_file, &cfg_lst, &dir_lst, &valid);

	if(valid)	USER(FG_GREEN "config file ok\n" RESET_ATTR);
	else		USER(FG_RED "syntax error detected\n" RESET_ATTR);

	list_for_each(cfg_lst, cfg){
		USER("config: %s\n"
			 "\tlogfile: %s\n"
			 "\toutdir: %s\n"
			 "\ttmpdir: %s\n"
			 "\trsyncdir: %s\n"
			 "\tarchive: %d\n"
			 "\tpreserve: %d\n"
			 "\tindicate: %d\n"
			 "\tverbosity: %d\n"
			 "\n",
			 cfg->name,
			 cfg->log_file,
			 cfg->out_dir,
			 cfg->tmp_dir,
			 cfg->rsync_dir,
			 cfg->archive,
			 cfg->preserve,
			 cfg->indicate,
			 cfg->verbosity
		);

		list_rm(&cfg_lst, cfg);
		delete cfg;
	}

	list_for_each(dir_lst, dir){
		USER("directory: %s\n", dir->path);

		list_for_each(dir->file_lst, file)
			USER("\tfile: %s %s\n", file->name, file->rsync_dir);

		USER("\n");

		list_rm(&dir_lst, dir);
		delete dir;
	}

	log::cleanup();
	return 0;
}
