#include <config/config.h>
#include <common/log.h>
#include <common/list.h>
#include <common/escape.h>
#include <common/string.h>
#include <cfg/parser.tab.h>
#include <main/argv.h>
#include <main/shell.h>
#include <main/ui.h>
#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>


/* macros */
#define MAXLEN	256

/* local/static prototypes */
static cfg_t *cfg_apply(cfg_t *lst);
static void backup(cfg_t *cfg, dir_t *dir_lst);
static void restore(cfg_t *cfg, dir_t *dir_lst);


/* global functions */
int main(int argc, char **argv){
	int ret;
	bool valid;
	char name[MAXLEN];
	cfg_t *cfg_lst,
		  *cit,
		  *cfg;
	dir_t *dir_lst,
		  *dit;
	termios term_s, term_b;


	ret = 1;
	cfg_lst = 0;
	dir_lst = 0;

	/* parse command line */
	if(argv::parse(argc, argv) != 0)
		return 1;

	if(argv::set.verbosity)
		log::set_log_level(argv::verbosity);

	/* prepare terminal paramter */
	tcgetattr(0, &term_s);
	tcgetattr(0, &term_b);

	term_s.c_lflag &= ~(ECHO | ICANON);

	tcsetattr(0, TCSAFLUSH, &term_s);

	/* untar archive if restore from archive */
	if(argv::restore && ftype("", argv::archive) == FTYPE_FILE){
		// force default tmp-dir to be used, to avoid extrating
		// the backup archive to the default directory while using
		// the directory from the config file afterwards
		argv::set.tmp_dir = 1;

		if(mkdir("", argv::tmp_dir, false) != 0)
			goto cleanup;

		if(tar("xzf", argv::archive, argv::tmp_dir, "", false) != 0){
			rmdir(argv::tmp_dir, false);
			goto cleanup;
		}
	}

	/* parse config file */
	if(argv::restore){
		switch(ftype("", argv::archive)){
		case FTYPE_DIR:
			snprintf(name, MAXLEN, "%s%sconfig.bc", argv::archive, (argv::archive[strlen(argv::archive) - 1] != '/' ? "/" : ""));
			break;

		case FTYPE_FILE:
			snprintf(name, MAXLEN, "%sconfig.bc", argv::tmp_dir);
			break;

		default:
			goto cleanup;
		}
	}
	else
		snprintf(name, MAXLEN, argv::config_file);

	USER("parsing configuration from %s " POSSAFE "\n", name);

	if(cfgparse(name, &cfg_lst, &dir_lst, &valid) != 0 || !valid)
		goto cleanup;

	USER(POSRESTORE POSUP);
	USEROK();

	/* synchronise config and command line */
	cfg = cfg_apply(cfg_lst);

	if(cfg == 0)
		goto cleanup;

	log::set_log_file(cfg->log_file);
	log::set_log_level(cfg->verbosity);

	/* print command line and config */
	USERHEAD("selected configuration");
	CFG_PRINT(USER, cfg);

	USER2HEAD("parsed command line arguments");
	ARGV_PRINT(USER2);

	USER2HEAD("parsed configurations");
	list_for_each(cfg_lst, cit)
		CFG_PRINT(USER2, cit);

	USER2HEAD("parsed directories");
	list_for_each(dir_lst, dit)
		DIR_PRINT(dit, USER2);

	/* check UID */
	if(geteuid() != 0)
		USER(FG_YELLOW "executing as none-root user, some files might not be accessible" RESET_ATTR "\n");

	/* main functions */
	if(argv::restore)	restore(cfg, dir_lst);
	else				backup(cfg, dir_lst);

	USERHEAD("[cleaning up]");

	/* delete tmp directory */
	if(!cfg->preserve)
		rmdir(cfg->tmp_dir, cfg->indicate);

	/* flush buffers */
	USER("flushing file system caches ");

	if(file_write("/proc/sys/vm/drop_caches", "w", "3\n") != 0)
		USERERR("/proc/sys/vm/drop_caches: %s", strerror(errno));
	else
		USEROK();

	ret = 0;

cleanup:
	/* reset terminal paramter */
	tcsetattr(0, TCSAFLUSH, &term_b);

	/* cleanup */
	list_for_each(cfg_lst, cit){
		list_rm(&cfg_lst, cit);
		delete cit;
	}

	list_for_each(dir_lst, dit){
		list_rm(&dir_lst, dit);
		delete dit;
	}

	log::cleanup();
	argv::cleanup();

	return ret;
}


/* local functions */
cfg_t *cfg_apply(cfg_t *lst){
	cfg_t *cfg;


	cfg = list_find_str(lst, name, argv::config);

	if(cfg == 0){
		ERROR("unknown configuration \"%s\"\n", argv::config);
		return 0;
	}

	if(argv::set.out_dir || cfg->out_dir == 0)
		XCHG(cfg->out_dir, argv::out_dir);

	if(argv::set.tmp_dir || cfg->tmp_dir == 0)
		XCHG(cfg->tmp_dir, argv::tmp_dir);

	if(argv::set.log_file || cfg->log_file == 0){
		delete [] cfg->log_file;
		cfg->log_file = stralloc(argv::log_file);
	}

	if(cfg->rsync_dir == 0)
		cfg->rsync_dir = stralloc(cfg->out_dir, strlen(cfg->out_dir));

	if(argv::archive != 0)	cfg->archive = true;
	if(argv::set.indicate)	cfg->indicate = argv::indicate;
	if(argv::set.preserve)	cfg->preserve = argv::preserve;
	if(argv::set.noconfig)	cfg->noconfig = argv::noconfig;
	if(argv::set.nodata)	cfg->nodata = argv::nodata;
	if(argv::set.verbosity)	cfg->verbosity = argv::verbosity;

	return cfg;
}

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
	if(copy("", argv::config_file, "", cfg->tmp_dir, "config.bc", CMD_COPY, cfg->indicate) != 0)
		return;

	/* create backup.date in tmp directory */
	USER("generating backup.date ");

	if(file_write("backup.date", "w", "%s\n", log::stime()) != 0){
		USERERR("backup.date: %s", strerror(errno));
		return;
	}

	USEROK();

	if(copy("", "backup.date", "", cfg->tmp_dir, "", CMD_MOVE, cfg->indicate) != 0)
		return;

	/* cp files w/o rsync directory */
	if(!cfg->noconfig){
		USERHEAD("[processing 'normal' files (files w/o rsync directory)");

		list_for_each(dir_lst, dir){
			list_for_each(dir->file_lst, file){
				if(file->rsync_dir == 0){
					dst = dirname(dir->path, file->name);

					copy(dir->path, file->name, cfg->tmp_dir, (dst[0] == '/') ? dst + 1 : dst, "", CMD_COPY, cfg->indicate);

					delete [] dst;
				}
			}
		}

		/* cp to out directory or generate archive */
		if(mkdir(0, cfg->out_dir, cfg->indicate) != 0)
			return;

		if(cfg->archive){
			snprintf(name, MAXLEN, "%sbackup_%s.tar.gz", cfg->out_dir, log::stime());

			USERHEAD("[creating backup archive \"%s\"]", name);
			tar("czf", name, cfg->tmp_dir, ".", cfg->indicate);
		}
		else{
			USERHEAD("[copy to output directory \"%s\"]", cfg->tmp_dir);
			copy(cfg->tmp_dir, "*", "", cfg->out_dir, "", CMD_RSYNC, cfg->indicate);
		}
	}

	/* cp files with rsync directory */
	if(!cfg->nodata){
		USERHEAD("[processing 'rsync' files (files with rsync directory)]");

		list_for_each(dir_lst, dir){
			list_for_each(dir->file_lst, file){
				if(file->rsync_dir != 0)
					copy(dir->path, file->name, cfg->rsync_dir, file->rsync_dir, "", CMD_RSYNC, cfg->indicate);
			}
		}
	}
}

/**
 * TODO
 * 	- restore from archive or directory (change help message)
 * 	- copy files
 * 		- ask for every file to
 * 			- copy/all		c/C
 * 			- move/all		m/M
 * 			- skip/all		s/S
 * 			- show diff/all	d/D
 *
 * 	- add 'perform for all files' option
 */
void restore(cfg_t *cfg, dir_t *dir_lst){
	/* check user */
	if(!yesno("continue restore?"))
		return;


	USERHEAD("[restore from \"%s\"]", argv::archive);

	if(!cfg->noconfig){
	}

	if(!cfg->nodata){
	}
}
