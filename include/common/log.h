#ifndef LOG_H
#define LOG_H


#include <config/config.h>
#include <sys/types.h>
#include <stdio.h>


using namespace std;


/* types */
enum log_level_t{
	NONE = 0x0,
	ERROR = 0x1,
	USER = 0x2,
	TODO = 0x4,
};


/* macros */
#define ERROR(msg, ...)	log::print(ERROR, msg, ##__VA_ARGS__)
#define TODO(msg, ...)	log::print(TODO,	"[TBD] %15.15s:%-5d %15.15s(): " msg, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define USER(msg, ...)	log::print(USER, msg, ##__VA_ARGS__)

#define LOG_LEVEL	((log_level_t)(USER | ERROR))


/* class */
class log{
public:
	/* init and cleanup function */
	static int init(char const *file_name, log_level_t lvl);
	static void cleanup();

	/* add entry to log */
	static void print(log_level_t lvl, char const *msg, ...);

	/* get current time/date */
	static char *stime();

private:
	static FILE *log_file;			// file pointer to log file
	static log_level_t log_level;	// current log level
};


#endif
