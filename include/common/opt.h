#ifndef OPT_H
#define OPT_H


#include <common/log.h>
#include <stdarg.h>


/* types */
typedef enum{
	OPT_CONFIG_FILE = 0,
	OPT_CONFIG,
	OPT_TMP_DIR,
	OPT_RESTORE,
	OPT_ARCHIVE,
	OPT_INDICATE,
	OPT_PRESERVE,
	OPT_LOG_FILE,
	OPT_VERBOSE,
	OPT_HELP,
	OPT_VERSION,
} opt_id_t;

typedef struct{
	opt_id_t id;
	char const *long_name,
			   *short_name,
			   *arg,
			   *help;
} opt_desc_t;

struct opt_t{
	char const *name;
	opt_desc_t const *desc;
};

typedef opt_t opt_t;


#endif // OPT_H
