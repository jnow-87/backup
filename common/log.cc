#include <config/config.h>
#include <common/log.h>
#include <stdarg.h>
#include <unistd.h>
#include <time.h>


/* static variables */
FILE *log::log_file = 0;
log_level_t log::log_level = NONE;


/* class definition */
/**
 * \brief	init log system
 *
 * \param	file_name	file name of log file
 * \param	lvl			log level to apply
 *
 * \return	0			success
 * 			-1			error (check errno)
 */
int log::init(char const *file_name, log_level_t lvl){
	log_level = lvl;

	if(log_file == 0 && file_name != 0){
		log_file = fopen(file_name, "w");

		if(log_file == 0)
			return -1;
	}

	return 0;
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
 * \param	...			arguments as defined in <msg>
 */
void log::print(log_level_t lvl, char const *msg, ...){
	va_list lst;


	if(lvl & log_level){
		va_start(lst, msg);

		if(lvl & ERROR)	vfprintf(stderr, msg, lst);
		else			vprintf(msg, lst);

		va_end(lst);
	}

	if(log_file){
		fprintf(log_file, "[%19.19s]", stime());

		va_start(lst, msg);
		vfprintf(log_file, msg, lst);
		fflush(log_file);
		va_end(lst);
	}
}

/**
 * \brief	get current time/date as string
 *
 * \return	pointer to time/date string - string is allocated statically and
 * 			hence must not be freed
 */
char *log::stime(){
	static char s[80];
	time_t t;
	tm *ts;


	t = time(0);
	ts = localtime(&t);
	strftime(s, 80, "%d.%m.%Y %H:%M:%S", ts);

	return s;
}
