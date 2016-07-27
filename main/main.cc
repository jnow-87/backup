#include <config/config.h>
#include <common/log.h>
#include <common/argv.h>
#include <common/list.h>
#include <common/escape.h>
#include <common/string.h>
#include <cfg/parser.tab.h>
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
	cfg_t *cfg_lst,
		  *cit,
		  *cfg;
	dir_t *dir_lst,
		  *dit;
	termios term_s, term_b;


	ret = 1;

	/* parse command line */
	if(argv::parse(argc, argv) != 0)
		return 1;

	if(argv::set.verbosity)
		log::set_log_level(argv::verbosity);

	/* parse config file */
	if(cfgparse(argv::config_file, &cfg_lst, &dir_lst, &valid) != 0 || !valid)
		goto cleanup;

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

	/* set terminal paramter */
	tcgetattr(0, &term_s);
	tcgetattr(0, &term_b);

	term_s.c_lflag &= ~(ICANON);

	tcsetattr(0, TCSAFLUSH, &term_s);

	/* main functions */
	/* TODO
	 * cmd line
	 * 	-r	restore from archive
	 * 	-a	backup with archive
	 * 	-	backup with copy
	 *
	 * config
	 * 	.archive = 1	backup with archive
	 * 	.archive = 0	backup with copy
	 */

	if(argv::restore)	restore(cfg, dir_lst);
	else				backup(cfg, dir_lst);

	/* reset terminal paramter */
	tcsetattr(0, TCSAFLUSH, &term_b);

	ret = 0;

cleanup:
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

	return ret;
}


/* local functions */
cfg_t *cfg_apply(cfg_t *lst){
#define REALLOC(s, default){ \
	if(argv::s){ \
		delete [] cfg->s; \
		cfg->s = DIRALLOC(argv::s, strlen(argv::s)); \
	} \
	\
	if(cfg->s == 0) \
		cfg->s = DIRALLOC((char*)default, strlen(default)); \
}

	cfg_t *cfg;


	cfg = list_find_str(lst, name, argv::config);

	if(cfg == 0){
		ERROR("unknown configuration \"%s\"\n", argv::config);
		return 0;
	}

	REALLOC(out_dir, CONFIG_OUT_DIR);
	REALLOC(tmp_dir, CONFIG_TMP_DIR);
	REALLOC(log_file, CONFIG_LOG_FILE);

	if(cfg->log_file[strlen(cfg->log_file) - 1] == '/')
		cfg->log_file[strlen(cfg->log_file) - 1] = 0;

	if(cfg->rsync_dir == 0)
		cfg->rsync_dir = stralloc(cfg->out_dir, strlen(cfg->out_dir));

	if(argv::archive != 0)	cfg->archive = true;
	if(argv::set.indicate)	cfg->indicate = argv::indicate;
	if(argv::set.preserve)	cfg->preserve = argv::preserve;
	if(argv::set.verbosity)	cfg->verbosity = argv::verbosity;

	return cfg;

#undef REALLOC
}

/**
 * TODO
 * 	- main functionality
 * 		- rsync_dir is relative to out_dir or absolute
 * 		- options: --noconfig --nodata
 * 		- handle "special" directories like "homes"
 *
 */
void backup(cfg_t *cfg, dir_t *dir_lst){
	FILE *fp;
	char name[MAXLEN];
	char *dst;
	dir_t *dir;
	file_t *file;


	/* check user */
	if(!yesno("continue backup?"))
		return;

	/* init tmp directory */
	if(rmdir(cfg->tmp_dir, cfg->indicate) != 0)		return;
	if(mkdir(0, cfg->tmp_dir, cfg->indicate) != 0)	return;

	/* cp config to tmp directory */
	if(copy("", argv::config_file, "", cfg->tmp_dir, CMD_COPY, cfg->indicate) != 0)
		return;

	/* create backup.date in tmp directory */
	USER("generating backup.date ");
	fp = fopen("backup.date", "w");

	if(fp == 0){
		USERERR("backup.date: %s", strerror(errno));
		goto clean;
	}

	fprintf(fp, "%s\n", log::stime());
	fclose(fp);

	USEROK();

	if(copy("", "backup.date", "", cfg->tmp_dir, CMD_MOVE, cfg->indicate) != 0)
		goto clean;

	/* cp files w/o rsync directory */
	USERHEAD("[processing 'normal' files (files w/o rsync directory)");

	list_for_each(dir_lst, dir){
		list_for_each(dir->file_lst, file){
			// TODO
			// 	ensure file.name does not end on '/'
			// 	combine dir->path and file->name, removing the file
			if(file->rsync_dir == 0){
				dst = dirname(dir->path, file->name);

				copy(dir->path, file->name, cfg->tmp_dir, (dst[0] == '/') ? dst + 1 : dst, CMD_COPY, cfg->indicate);

				delete [] dst;
			}
		}
	}

	/* cp to out directory or generate archive */
	if(mkdir(0, cfg->out_dir, cfg->indicate) != 0)
		return;

	if(cfg->archive){
		snprintf(name, MAXLEN, "backup_%s.tar.gz", log::stime());

		USERHEAD("[creating backup archive \"%s\"]", name);
		tar("czf", cfg->tmp_dir, cfg->out_dir, name, cfg->indicate);
	}
	else{
		USERHEAD("[copy to output directory \"%s\"]", cfg->tmp_dir);
		copy(cfg->tmp_dir, "*", "", cfg->out_dir, CMD_RSYNC, cfg->indicate);
	}

	/* cp files with rsync directory */
	USERHEAD("[processing 'rsync' files (files with rsync directory)");

	list_for_each(dir_lst, dir){
		list_for_each(dir->file_lst, file){
			if(file->rsync_dir != 0)
				copy(dir->path, file->name, cfg->rsync_dir, file->rsync_dir, CMD_RSYNC, cfg->indicate);
		}
	}

clean:
	/* delete tmp directory */
	if(!cfg->preserve)
		rmdir(cfg->tmp_dir, cfg->indicate);
}

/**
 * TODO
 * 	- restore from archive or directory (change help message)
 * 	- copy files
 * 		- ask for every file to
 * 			- copy
 * 			- move
 * 			- skip
 * 			- show diff
 *
 * 	- add 'perform for all files' option
 */
void restore(cfg_t *cfg, dir_t *dir_lst){
	/* check user */
	if(!yesno("continue restore?"))
		return;


	USERHEAD("[restore from \"%s\"]", argv::archive);
	// TODO
}
