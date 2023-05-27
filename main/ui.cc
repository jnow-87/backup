#include <common/log.h>
#include <main/argv.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>


/* global functions */
bool yesno(char const *text){
	char c;


	if(argv::set.batch_mode)
		return true;

	if(text == 0)
		return false;

	USER("%s [y/N]\n", text);

	while(fread(&c, 1, 1, stdin) != 1)
		usleep(100000);

	if(c == 'y' || c == 'Y')
		return true;
	return false;
}

char uinput(char const *text, char const *valid, ...){
	char c;
	va_list lst;


	if(text != 0){
		va_start(lst, valid);
		log::vprint(USER0, text, lst);
		va_end(lst);
	}
	else
		ERROR("invalid text\n");

	while(1){
		if(fread(&c, 1, 1, stdin) == 1){
			if(strchr(valid, c) != 0)
				return c;
		}
		else
			usleep(100000);
	}
}
