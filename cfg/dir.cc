#include <common/list.h>
#include <cfg/dir.h>
#include <cfg/file.h>

dir_t::dir_t(){
	path = 0;
	file_lst = 0;
}

dir_t::~dir_t(){
	file_t *file;


	delete [] path;

	list_for_each(file_lst, file){
		list_rm(&file_lst, file);
		delete file;
	}
}
