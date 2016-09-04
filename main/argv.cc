#include <config/config.h>
#include <common/escape.h>
#include <common/string.h>
#include <main/opt.h>
#include <main/opt.hash.h>
#include <main/argv.h>
#include <version.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


/* macros */
/**
 * \brief	check if an additional, non-option argument value is available
 * 			if true, increment counter and return the argument
 * 			otherwise print the help message and exit
 *
 * \return	next, non-option argument value
 */
#define GET_ARG()({ \
	if(i + 1 == argc || opt_tbl::lookup(argv[i + 1], strlen(argv[i + 1]))) \
		argv::help(argv[0], "missing parameter for option \"%s\"", argv[i]); \
	\
	argv[++i]; \
})


/* class definition */
char const *argv::config_file = CONFIG_CONFIG_FILE;
char const *argv::config = CONFIG_CONFIG;
char *argv::out_dir = 0;
char *argv::tmp_dir = 0;
char *argv::archive = 0;
bool argv::restore = false;
bool argv::indicate = false;
bool argv::preserve = false;
bool argv::noconfig = false;
bool argv::nodata = false;
char const *argv::log_file = CONFIG_LOG_FILE;
unsigned int argv::verbosity = 0;
argv::set_t argv::set = { 0 };


/**
 * \brief	parse command line arguments
 *
 * \param	argc	number of arguments
 * \param	argv	argument values
 *
 * \return	0 on success
 * 			exit(1) otherwise
 */
int argv::parse(int argc, char **argv){
	int i;
	struct opt_t const *opt;
	opt_desc_t const *desc;


	for(i=1; i<argc; ++i){
		opt = opt_tbl::lookup(argv[i], strlen(argv[i]));

		if(opt == 0)
			help(argv[0], "invalid command line option \"%s\"", argv[i]);

		desc = opt->desc;

		switch(desc->id){
		case OPT_CONFIG_FILE:
			config_file = GET_ARG();
			break;

		case OPT_CONFIG:
			config = GET_ARG();
			break;

		case OPT_OUT_DIR:
			set.out_dir = 1;
			out_dir = stralloc(GET_ARG(), PF_DIR);
			break;

		case OPT_TMP_DIR:
			set.tmp_dir = 1;
			tmp_dir = stralloc(GET_ARG(), PF_DIR);
			break;

		case OPT_RESTORE:
			restore = true;
			archive = stralloc(GET_ARG(), 0);
			break;

		case OPT_ARCHIVE:
			archive = stralloc("", 0);
			break;

		case OPT_INDICATE:
			set.indicate = 1;
			indicate = true;
			break;

		case OPT_PRESERVE:
			set.preserve = 1;
			preserve = true;
			break;

		case OPT_NOCONFIG:
			set.noconfig = 1;
			noconfig = true;
			break;

		case OPT_DOCONFIG:
			set.noconfig = 1;
			noconfig = false;
			break;

		case OPT_NODATA:
			set.nodata = 1;
			nodata = true;
			break;

		case OPT_DODATA:
			set.nodata = 1;
			nodata = false;
			break;

		case OPT_LOG_FILE:
			set.log_file = 1;
			log_file = GET_ARG();
			break;

		case OPT_VERBOSE:
			set.verbosity = 1;
			verbosity = atoi(GET_ARG());
			break;

		case OPT_HELP:
			help(argv[0]);
			break;

		case OPT_VERSION:
			USER("version info:\n" VERSION "\n");
			exit(0);
		}
	}

	if(tmp_dir == 0)	tmp_dir = stralloc(CONFIG_TMP_DIR, PF_DIR);
	if(out_dir == 0)	out_dir = stralloc(CONFIG_OUT_DIR, PF_DIR);

	return 0;
}

/**
 * \brief	free memory
 */
void argv::cleanup(){
	delete [] tmp_dir;
	delete [] out_dir;
	delete [] archive;
}

/**
 * \brief	print the help message
 *
 * \param	pname	program name
 * \param	msg		optional error message format string
 * \param	...		optional arguments
 */
void argv::help(char *pname, char const *msg, ...){
	unsigned int i;
	struct opt_t const *opt;
	opt_desc_t const *desc;
	va_list lst;


	/* print message */
	if(msg){
		va_start(lst, msg);
		ERROR(FG_RED "\t");
		VERROR(msg, lst);
		ERROR(RESET_ATTR "\n\n");
		va_end(lst);
	}

	/* print header */
	USER("usage: %s <options>\n\noptions:\n", pname);

	/* print options and help messages */
	for(i=opt_tbl::MIN_HASH_VALUE; i<=opt_tbl::MAX_HASH_VALUE; ++i){
		opt = opt_tbl::wordlist + i;
		desc = opt->desc;

		if(opt->name != 0 && strncmp(opt->name, "--", 2) == 0)
			USER("\t%15.15s%s%2s %-10.10s\t%s\n", desc->long_name, (desc->short_name[0] ? ", " : ""), desc->short_name, desc->arg, desc->help);
	}

	/* print default settings */
	USER("\ndefault settings:\n"
		   "\t%15.15s\t%s\n"
		   "\t%15.15s\t%s\n",
		   "config-file", CONFIG_CONFIG_FILE,
		   "log-file", CONFIG_LOG_FILE
	);

	if(msg)
		exit(1);

	exit(0);
}
