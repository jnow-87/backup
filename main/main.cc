#include <config/config.h>
#include <common/log.h>
#include <common/argv.h>
#include <common/list.h>
#include <common/escape.h>
#include <common/string.h>
#include <cfg/parser.tab.h>
#include <stdio.h>
#include <string.h>


/* local/static prototypes */
static cfg_t *cfg_apply(cfg_t *lst);


/**
 * TODO
 * 	- main functionality
 * 		- rsync_dir is relative to out_dir or absolute
 * 		- options: handle_normale handle_rsync
 * 		- handle "special" directories like "homes"
 *
 *	- config variables resolved later
 */

/* global functions */
int main(int argc, char **argv){
	int ret;
	bool valid;
	cfg_t *cfg_lst,
		  *cit,
		  *cfg;
	dir_t *dir_lst,
		  *dit;


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
	USER(FG_VIOLETT "selected configuration" RESET_ATTR "\n");
	CFG_PRINT(USER, cfg);

	USER2(FG_VIOLETT "parsed command line arguments" RESET_ATTR "\n");
	ARGV_PRINT(USER2);

	USER2(FG_VIOLETT "parsed configurations" RESET_ATTR "\n");
	list_for_each(cfg_lst, cit)
		CFG_PRINT(USER2, cit);

	USER2(FG_VIOLETT "parsed directories" RESET_ATTR "\n");
	list_for_each(dir_lst, dit)
		DIR_PRINT(dit, USER2);

	/* main functions */
	if(cfg->backup){
		// TODO backup
	}
	else{
		// TODO restore
	}

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
		cfg->s = stralloc(argv::s, strlen(argv::s)); \
	} \
	\
	if(cfg->s == 0) \
		cfg->s = stralloc((char*)default, strlen(default)); \
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

	if(argv::set.backup)	cfg->backup = argv::backup;
	if(argv::set.indicate)	cfg->indicate = argv::indicate;
	if(argv::set.preserve)	cfg->preserve = argv::preserve;
	if(argv::set.verbosity)	cfg->verbosity = argv::verbosity;

	return cfg;

#undef REALLOC
}
