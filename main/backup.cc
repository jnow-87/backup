#include <common/log.h>
#include <common/list.h>
#include <common/string.h>
#include <cfg/cfg.h>
#include <cfg/dir.h>
#include <cfg/file.h>
#include <main/ui.h>
#include <main/shell.h>
#include <main/argv.h>
#include <errno.h>
#include <string.h>


/**
 * TODO
 * 	- main functionality
 * 		- handle "special" directories like "homes"
 *
 */
void backup(cfg_t *cfg, dir_t *dir_lst){
	char name[MAXLEN];
	char *dst;
	dir_t *dir;
	file_t *file;


	/* check user */
	if(!yesno("continue backup?"))
		return;

	/* remove tmp directory */
	if(rmdir(cfg->tmp_dir, cfg->indicate) != 0)
		return;

	/* cp config to tmp directory */
	if(copy("", "", argv::config_file, "", cfg->tmp_dir, "config.bc", CMD_COPY, cfg->indicate) != 0)
		return;

	/* create backup.date in tmp directory */
	USER("generating backup.date ");

	if(file_write("backup.date", "w", "%s\n", log::stime()) != 0){
		USERERR("backup.date: %s", strerror(errno));
		return;
	}

	USEROK();

	if(copy("", "", "backup.date", "", cfg->tmp_dir, "", CMD_MOVE, cfg->indicate) != 0)
		return;

	/* cp files w/o rsync directory */
	if(!cfg->noconfig){
		USERHEAD("[processing config files (files w/o rsync directory)");

		list_for_each(dir_lst, dir){
			list_for_each(dir->file_lst, file){
				if(file->rsync_dir == 0){
					// use dirname, avoid generating too many sub-directories when copying directories
					// e.g. cp -r /base/dir/ /tmp/base/dir creates the output directory /tmp/base/dir/dir
					dst = dirname(dir->path, file->name);

					copy("", dir->path, file->name, cfg->tmp_dir, dst, "", CMD_COPY, cfg->indicate);

					delete [] dst;
				}
			}
		}

		/* cp to out directory or generate archive */
		if(mkdir("", cfg->out_dir, cfg->indicate) != 0)
			return;

		if(cfg->archive){
			snprintf(name, MAXLEN, "%sbackup_%s.tar.gz", cfg->out_dir, log::stime());

			USERHEAD("[creating backup archive \"%s\"]", name);
			tar("czf", name, cfg->tmp_dir, ".", cfg->indicate);
		}
		else{
			USERHEAD("[copy to output directory \"%s\"]", cfg->tmp_dir);
			copy("", cfg->tmp_dir, "*", "", cfg->out_dir, "", CMD_RSYNC, cfg->indicate);
		}
	}

	/* cp files with rsync directory */
	if(!cfg->nodata){
		USERHEAD("[processing data files (files with rsync directory)]");

		list_for_each(dir_lst, dir){
			list_for_each(dir->file_lst, file){
				if(file->rsync_dir != 0)
					copy("", dir->path, file->name, cfg->rsync_dir, file->rsync_dir, "", CMD_RSYNC, cfg->indicate);
			}
		}
	}
}
