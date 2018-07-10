#include <config/config.h>
#include <common/log.h>
#include <common/list.h>
#include <common/string.h>
#include <cfg/parser.tab.h>
#include <main/argv.h>
#include <main/shell.h>
#include <main/ui.h>
#include <main/backup.h>
#include <main/restore.h>
#include <unistd.h>
#include <termios.h>


/* local/static prototypes */
static cfg_t *cfg_apply(cfg_t *lst);


/* global functions */
int main(int argc, char **argv){
	int ret;
	bool valid;
	char cfg_name[MAXLEN];
	cfg_t *cfg_lst,
		  *cit,
		  *cfg;
	dir_t *dir_lst,
		  *dit;
	script_t *sit;
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

	/* prepare restore and identify config file name */
	if(argv::restore){
		switch(ftype("", "", argv::archive)){
		case FTYPE_FILE:
			// expect tar.gz archive
			snprintf(cfg_name, MAXLEN, "%sconfig.bc", argv::tmp_dir);

			// remove tmp directory
			if(rmdir(argv::tmp_dir, false) != 0)
				goto cleanup;

			if(mkdir("", argv::tmp_dir, false) != 0)
				goto cleanup;

			// untar archive
			if(tar("xzf", argv::archive, argv::tmp_dir, "", false) != 0){
				rmdir(argv::tmp_dir, false);
				goto cleanup;
			}

			// force default tmp-dir to be used, to avoid extrating
			// the backup archive to the default directory while using
			// the directory from the config file afterwards
			argv::set.tmp_dir = 1;
			break;

		case FTYPE_DIR:
			// expect directory containing config.bc
			snprintf(cfg_name, MAXLEN, "%s%sconfig.bc", argv::archive, (STRLASTC(argv::archive) != '/' ? "/" : ""));

			// ensure restore() can use argv::tmp_dir as source
			if(argv::tmp_dir)
				delete argv::tmp_dir;

			argv::tmp_dir = stralloc(argv::archive, PF_DIR);
			argv::set.tmp_dir = 1;
			break;

		default:
			ERROR("expecting archive or config file for restore\n");
			goto cleanup;
		}
	}
	else
		snprintf(cfg_name, MAXLEN, argv::config_file);

	/* parse config file */
	USER("parsing configuration from %s\n", cfg_name);

	if(cfgparse(cfg_name, &cfg_lst, &dir_lst, &valid) != 0 || !valid)
		goto cleanup;

	/* synchronise config and command line */
	cfg = cfg_apply(cfg_lst);

	if(cfg == 0)
		goto cleanup;

	if(log::set_log_file(cfg->log_file) != 0)
		ERROR("unable to open log-file \"%s\" (%s)\n", cfg->log_file, strerror(errno));

	log::set_log_level(cfg->verbosity);

	/* print command line and config */
	USERHEAD("selected configuration");
	CFG_PRINT(cfg, USER);

	USER2HEAD("parsed command line arguments");
	ARGV_PRINT(USER2);

	USER2HEAD("parsed configurations");
	list_for_each(cfg_lst, cit)
		CFG_PRINT(cit, USER2);

	USER2HEAD("parsed directories");
	list_for_each(dir_lst, dit)
		DIR_PRINT(dit, USER2);

	if(argv::restore){
		USER2HEAD("parsed pre-restore scripts");
		list_for_each(cfg->pre_restore_lst, sit)
			USER2("\t%s\n", sit->cmd);

		USER2HEAD("parsed post-restore scripts");
		list_for_each(cfg->post_restore_lst, sit)
			USER2("\t%s\n", sit->cmd);
	}
	else{
		USER2HEAD("parsed pre-backup scripts");
		list_for_each(cfg->pre_backup_lst, sit)
			USER2("\t%s\n", sit->cmd);

		USER2HEAD("parsed post-backup scripts");
		list_for_each(cfg->post_backup_lst, sit)
			USER2("\t%s\n", sit->cmd);
	}

	/* check UID */
	if(geteuid() != 0)
		USER(FG_YELLOW "executing as none-root user, some files might not be accessible" RESET_ATTR "\n");

	/* main functions */
	if(argv::restore)	restore(cfg, dir_lst);
	else				backup(cfg, dir_lst);

	USERHEAD("[cleaning up]");

	/* delete tmp directory if its not the recovery archive */
	if(!cfg->preserve && (!argv::restore || (argv::restore && strncmp(cfg->tmp_dir, argv::archive, strlen(argv::archive)) != 0)))
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
		USER("valid configurations:");

		list_for_each(lst, cfg)
			USER(" %s", cfg->name);

		USER("\n");

		return 0;
	}

	if(argv::set.out_dir || cfg->out_dir == 0)
		XCHG(cfg->out_dir, argv::out_dir);

	if(argv::set.tmp_dir || cfg->tmp_dir == 0)
		XCHG(cfg->tmp_dir, argv::tmp_dir);

	if(argv::set.log_file || cfg->log_file == 0){
		delete [] cfg->log_file;
		cfg->log_file = stralloc(argv::log_file, PF_FILE);
	}

	if(cfg->rsync_dir == 0)
		cfg->rsync_dir = stralloc(cfg->out_dir, PF_DIR);

	if(argv::archive != 0)	cfg->archive = true;
	if(argv::set.indicate)	cfg->indicate = argv::indicate;
	if(argv::set.preserve)	cfg->preserve = argv::preserve;
	if(argv::set.noconfig)	cfg->noconfig = argv::noconfig;
	if(argv::set.nodata)	cfg->nodata = argv::nodata;
	if(argv::set.verbosity)	cfg->verbosity = argv::verbosity;

	return cfg;
}
