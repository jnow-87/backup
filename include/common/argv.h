#ifndef ARGV_H
#define ARGV_H


/* macros */
#define ARGV_PRINT() \
	USER1( \
		"%s: %s\n" \
		"%s: %s\n" \
		"%s: %s\n" \
		"%s: %s\n" \
		"%s: %s\n" \
		"%s: %d\n" \
		"%s: %d\n" \
		"%s: %u\n" \
		"%s: %s\n" \
		"%s: %u\n\n" \
		, \
		"config-file", argv::config_file, \
		"config", argv::config, \
		"out-dir", argv::out_dir, \
		"tmp-dir", argv::tmp_dir, \
		"archive", argv::archive, \
		"compress", argv::compress ? 1 : 0, \
		"indicate", argv::indicate ? 1 : 0, \
		"preserve", argv::preserve ? 1 : 0, \
		"log-file", argv::log_file, \
		"verbosity", argv::verbosity \
	);


/* class */
class argv{
public:
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

	static bool compress,
		 		indicate,
		 		preserve;

private:
	/* private functions */
	static void help(char *pname, char const *msg = 0, ...);
};


#endif // ARGV_H
