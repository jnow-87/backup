#ifndef UI_H
#define UI_H


#include <common/escape.h>
#include <common/log.h>
#include <stdarg.h>


/* macros */
#define USERINDICATE() \
	USER(FG_YELLOW "[INDICATE]" RESET_ATTR "\n");

#define USEROK() \
	USER(FG_GREEN "[OK]" RESET_ATTR "\n")

#define USERERR(msg, ...){ \
	USER(FG_RED "[ERR]"); \
	USER1(" (" msg ")", ##__VA_ARGS__); \
	USER(RESET_ATTR "\n"); \
}


#endif // UI_H
