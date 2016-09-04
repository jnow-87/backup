#include <config/config.h>
#include <common/log.h>
#include <stdarg.h>
#include <unistd.h>
#include <time.h>
#include <string.h>


/* static variables */
FILE *log::log_file = 0;
log_level_t log::log_level = ((log_level_t)(USER0 | WARN | ERROR));


/* class definition */
/**
 * \brief	open log file
 *
 * \param	name	file name of log file
 *
 * \return	0			success
 * 			-1			error (check errno)
 */
int log::set_log_file(char const *name){
	if(log_file == 0 && name != 0){
		log_file = fopen(name, "w");

		if(log_file == 0)
			return -1;
	}

	return 0;
}

/**
 * \brief	set log level
 *
 * \param	lvl		desired log level
 */
void log::set_log_level(unsigned int verbosity){
	switch(verbosity){
	default:
		// fall through

	case 3:
		log_level = (log_level_t)(log_level | TODO);
		// fall through

	case 2:
		log_level = (log_level_t)(log_level | USER2);
		// fall through

	case 1:
		log_level = (log_level_t)(log_level | USER0 | WARN | ERROR | USER1);
		break;

	case 0:
		log_level = (log_level_t)(USER0 | WARN | ERROR);
		break;
	}
}

/**
 * \brief	safely shut down log system
 */
void log::cleanup(){
	if(log_file != 0)
		fclose(log_file);

	log_file = 0;
}

/**
 * \brief	write log message to log file
 *
 * \param	lvl			log level of message
 * \param	msg			actual message (printf-like format string)
 * \param	...			variable length argument list
 */
void log::print(log_level_t lvl, char const *msg, ...){
	va_list lst;


	va_start(lst, msg);
	vprint(lvl, msg, lst);
	va_end(lst);
}


/**
 * \brief	write log message to log file
 *
 * \param	lvl			log level of message
 * \param	msg			actual message (printf-like format string)
 * \param	lst			variable length argument list
 */
void log::vprint(log_level_t lvl, char const *msg, va_list lst){
	va_list cp;
	FILE *fp;


	if(*msg == 0)
		return;

	va_copy(cp, lst);

	if(lvl & log_level){
		if(lvl & ERROR)	fp = stderr;
		else			fp = stdout;

		vfprintf(fp, msg, lst);
		fflush(fp);
	}

	if(log_file){
		fprintf(log_file, "[%19.19s]\t", stime());

		vfprintf(log_file, msg, cp);

		if(msg[strlen(msg) - 1] != '\n')
			fprintf(log_file, "\n");

		fflush(log_file);
	}
}

/**
 * \brief	get current time/date as string
 *
 * \return	pointer to time/date string - string is allocated statically and
 * 			hence must not be freed
 */
char *log::stime(void){
	static char s[80];
	time_t t;
	tm *ts;


	t = time(0);
	ts = localtime(&t);
	strftime(s, 80, "%Y.%m.%d@%H.%M.%S", ts);

	return s;
}
