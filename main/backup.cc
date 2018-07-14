#include <common/log.h>
#include <common/list.h>
#include <common/string.h>
#include <cfg/cfg.h>
#include <cfg/dir.h>
#include <cfg/file.h>
#include <cfg/script.h>
#include <main/ui.h>
#include <main/shell.h>
#include <main/argv.h>
#include <version.h>
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
	dir_t *dir;
	file_t *file;


	/* check user */
	if(!yesno("continue backup?"))
		return;

	/* execute pre-backup scripts */
	script_exec(cfg->pre_backup_lst, "pre-backup", cfg->indicate);

	/* cp files w/o rsync directory */
	if(!cfg->noconfig){
		USERHEAD("[prepare backup]");

		/* prepare tmp directory */
		if(rmdir(cfg->tmp_dir, false) != 0)
			return;

		if(mkdir("", cfg->tmp_dir, false) != 0)
			return;

		/* cp config to tmp directory */
		USER("copy config file %s ", argv::config_file);

		if(SHELL("cp %s %sconfig.bc", argv::config_file, cfg->tmp_dir) != 0){
			USERERR("%s", shellerrstr);
			return;
		}

		USEROK();

		/* create backup.info in tmp directory */
		USER("generating backup.info ");

		if(file_write("backup.info", "w",
			"date and time:\n\t%s\n\n"
			"version info:\n%s\n"
			,
			log::stime(),
			VERSION) != 0
		  ){
			USERERR("backup.info: %s", strerror(errno));
			return;
		}

		USEROK();

		if(copy("", "", "backup.info", "", cfg->tmp_dir, "", CMD_MOVE, cfg->indicate) != 0)
			return;

		/* copy files */
		USERHEAD("[processing config files (files w/o rsync directory)");

		list_for_each(dir_lst, dir){
			list_for_each(dir->file_lst, file){
				if(file->rsync_dir == 0)
					copy("", dir->path, file->name, cfg->tmp_dir, dir->path, file->name, CMD_COPY, cfg->indicate);
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

	/* execute post-backup scripts */
	script_exec(cfg->post_backup_lst, "post-backup", cfg->indicate);
}
