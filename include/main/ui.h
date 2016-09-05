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

#define USERERR(msg, ...)({ \
	USER(FG_RED "[ERR]"); \
	USER1(" (" msg ")", ##__VA_ARGS__); \
	USER(RESET_ATTR "\n"); \
})

#define USERHEAD(msg, ...) \
	USER("\n" FG_VIOLETT msg RESET_ATTR "\n", ##__VA_ARGS__)

#define USER1HEAD(msg, ...) \
	USER1("\n" FG_VIOLETT msg RESET_ATTR "\n", ##__VA_ARGS__)

#define USER2HEAD(msg, ...) \
	USER2("\n" FG_VIOLETT msg RESET_ATTR "\n", ##__VA_ARGS__)


/* prototypes */
bool yesno(char const *text);
char uinput(char const *text, char const *valid, ...);


#endif // UI_H
