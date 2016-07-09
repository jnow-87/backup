#include <cfg/file.h>


file_t::file_t(){
	name = 0;
	rsync_dir = 0;
}

file_t::~file_t(){
	delete [] name;
	delete [] rsync_dir;
}
