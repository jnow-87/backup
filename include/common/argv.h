#ifndef ARGV_H
#define ARGV_H


/* macros */
#define ARGV_PRINT(print) \
	print( \
		"\t%s: %s\n" \
		"\t%s: %s\n" \
		"\t%s: %s\n" \
		"\t%s: %s\n" \
		"\t%s: %s\n" \
		"\t%s: %s\n" \
		"\t%s: %d\n" \
		"\t%s: %d\n" \
		"\t%s: %u\n" \
		"\t%s: %u\n\n" \
		, \
		"config-file", argv::config_file, \
		"config", argv::config, \
		"log-file", argv::log_file ? argv::log_file : "-", \
		"out-dir", argv::out_dir ? argv::out_dir : "-", \
		"tmp-dir", argv::tmp_dir ? argv::tmp_dir : "-", \
		"archive", argv::archive ? argv::archive : "-", \
		"backup", argv::backup, \
		"indicate", argv::indicate, \
		"preserve", argv::preserve, \
		"verbosity", argv::verbosity \
	);


/* class */
class argv{
public:
	/* types */
	typedef struct{
		unsigned int verbosity:1,
					 backup:1,
					 indicate:1,
					 preserve:1;
	} set_t;

	/* public functions */
	static int parse(int argc, char **argv);

	/* public variables */
	static char *prog_name;

	static char *log_file;
	static unsigned int verbosity;

	static char *config_file,
		 		*config;

	static char *out_dir,
				*tmp_dir;

	static char *archive;

	static bool backup,
		 		indicate,
		 		preserve;

	static set_t set;

private:
	/* private functions */
	static void help(char *pname, char const *msg = 0, ...);
};


#endif // ARGV_H
