#ifndef LOG_H
#define LOG_H


#include <config/config.h>
#include <common/escape.h>
#include <sys/types.h>
#include <stdio.h>


using namespace std;


/* types */
enum log_level_t{
	NONE = 0x0,
	ERROR = 0x1,
	WARN = 0x2,
	USER0 = 0x4,
	USER1 = 0x8,
	USER2 = 0x10,
	TODO = 0x20,
	LOGONLY = 0x40,
};


/* macros */
#define ERROR(msg, ...)		log::print(ERROR, msg, ##__VA_ARGS__)
#define VERROR(msg, lst)	log::vprint(ERROR, msg, lst)
#define WARN(msg, ...)		log::print(WARN, FG_YELLOW "[WARN] " msg RESET_ATTR, ##__VA_ARGS__)
#define VWARN(msg, lst)		log::vprint(WARN, FG_YELLOW "[WARN] " msg RESET_ATTR, lst)
#define TODO(msg, ...)		log::print(TODO, "[TBD] %15.15s:%-5d %15.15s(): " msg, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define VTODO(msg, lst)		log::vprint(TODO, "[TBD] %15.15s:%-5d %15.15s(): " msg, __FILE__, __LINE__, __FUNCTION__, lst)
#define USER(msg, ...)		log::print(USER0, msg, ##__VA_ARGS__)
#define VUSER(msg, lst)		log::vprint(USER0, msg, lst)
#define USER1(msg, ...)		log::print(USER1, msg, ##__VA_ARGS__)
#define VUSER1(msg, lst)	log::vprint(USER1, msg, lst)
#define USER2(msg, ...)		log::print(USER2, msg, ##__VA_ARGS__)
#define VUSER2(msg, lst)	log::vprint(USER2, msg, lst)
#define LOGONLY(msg, ...)	log::print(LOGONLY, msg, ##__VA_ARGS__)
#define VLOGONLY(msg, lst)	log::vprint(LOGONLY, msg, lst)


/* class */
class log{
public:
	/* init and cleanup function */
	static int set_log_file(char const *name);
	static void set_log_level(unsigned int verbosity);
	static void cleanup();

	/* add entry to log */
	static void print(log_level_t lvl, char const *msg, ...);
	static void vprint(log_level_t lvl, char const *msg, va_list lst);

	/* get current time/date */
	static char *stime(void);

private:
	static FILE *log_file;			// file pointer to log file
	static log_level_t log_level;	// current log level
};


#endif
