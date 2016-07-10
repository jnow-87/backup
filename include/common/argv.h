#ifndef ARGV_H
#define ARGV_H


/* class */
class argv{
public:
	/* public functions */
	static int parse(int argc, char **argv);
	static void print(void);

	/* public variables */
	static char *prog_name;

	static char *log_file;
	static log_level_t log_level;

	static char *config_file,
		 		*config,
				*tmp_dir;

	static char *archive;

	static bool restore,
		 		compress,
		 		indicate,
		 		preserve;

private:
	/* private functions */
	static void help(char *pname, char const *msg = 0, ...);
};


#endif // ARGV_H
