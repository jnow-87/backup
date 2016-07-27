#include <common/log.h>
#include <unistd.h>


/* global functions */
bool yesno(char const *text){
	char c;


	if(text == 0)
		return false;

	USER("%s [y/N]\n", text);

	while(fread(&c, 1, 1, stdin) != 1)
		usleep(100000);

	if(c == 'y' || c == 'Y')
		return true;
	return false;
}
