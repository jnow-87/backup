#include <config/config.h>
#include <common/opt.h>
#include <common/opt.hash.h>
#include <common/argv.h>
#include <common/escape.h>
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
char *argv::config_file = (char*)CONFIG_CONFIG_FILE;
char *argv::config = (char*)CONFIG_CONFIG;
char *argv::out_dir = (char*)CONFIG_OUT_DIR;
char *argv::tmp_dir = (char*)CONFIG_TMP_DIR;
char *argv::archive = 0;
bool argv::compress = false;
bool argv::indicate = false;
bool argv::preserve = false;
char *argv::log_file = (char*)CONFIG_LOG_FILE;
log_level_t argv::log_level = LOG_LEVEL;


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
			argv::help(argv[0], "invalid command line option \"%s\"", argv[i]);

		desc = opt->desc;

		switch(desc->id){
		case OPT_CONFIG_FILE:
			argv::config_file = GET_ARG();
			break;

		case OPT_CONFIG:
			argv::config = GET_ARG();
			break;

		case OPT_OUT_DIR:
			out_dir = GET_ARG();
			break;

		case OPT_TMP_DIR:
			argv::tmp_dir = GET_ARG();
			break;

		case OPT_RESTORE:
			argv::compress = false;
			argv::archive = GET_ARG();
			break;

		case OPT_ARCHIVE:
			argv::compress = true;
			break;

		case OPT_INDICATE:
			argv::indicate = true;
			break;

		case OPT_PRESERVE:
			argv::preserve = true;
			break;

		case OPT_LOG_FILE:
			argv::log_file = GET_ARG();
			break;

		case OPT_VERBOSE:
			argv::log_level = (log_level_t)(argv::log_level | ERROR | TODO);
			break;

		case OPT_HELP:
			argv::help(argv[0]);
			break;

		case OPT_VERSION:
			printf("version info:\n" VERSION "\n");
			exit(0);
		}
	}

	return 0;
}


/**
 * \brief	print the current set of arguments
 */
void argv::print(void){
	printf("%s: %s\n"
		   "%s: %s\n"
		   "%s: %s\n"
		   "%s: %d\n"
		   "%s: %d\n"
		   "%s: %d\n"
		   "%s: %d\n"
		   "%s: %d\n"
		   "%s: %s\n"
		   "%s: 0x%x (%s (0x%x), %s (0x%x), %s (0x%x))\n"
		   ,
		   "config-file", config_file,
		   "config", config,
		   "tmp-dir", tmp_dir,
		   "archive", archive ? 1 : 0,
		   "restore", restore ? 1 : 0,
		   "compress", compress ? 1 : 0,
		   "indicate", indicate ? 1 : 0,
		   "preserve", preserve ? 1 : 0,
		   "log-file", log_file,
		   "log-level", log_level,
		   "ERROR", ERROR, "USER", USER, "TODO", TODO
	);
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
		printf(FG_RED "\t");
		vprintf(msg, lst);
		printf(RESET_ATTR "\n\n");
		va_end(lst);
	}

	/* print header */
	printf("usage: %s <options>\n\noptions:\n", pname);

	/* print options and help messages */
	for(i=opt_tbl::MIN_HASH_VALUE; i<=opt_tbl::MAX_HASH_VALUE; ++i){
		opt = opt_tbl::wordlist + i;
		desc = opt->desc;

		if(opt->name != 0 && strncmp(opt->name, "--", 2) == 0)
			printf("\t%15.15s%s%2s %-10.10s\t%s\n", desc->long_name, (desc->short_name[0] ? ", " : ""), desc->short_name, desc->arg, desc->help);
	}

	/* print default settings */
	printf("\ndefault settings:\n"
		   "\t%15.15s\t%s\n"
		   "\t%15.15s\t%s\n",
		   "config-file", CONFIG_CONFIG_FILE,
		   "log-file", CONFIG_LOG_FILE
	);

	if(msg)
		exit(1);

	exit(0);
}
