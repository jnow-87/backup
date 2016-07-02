#include <common/log.h>
#include <common/argv.h>
#include <stdio.h>


int main(int argc, char **argv){
	if(argv::parse(argc, argv) != 0)
		return 1;

	argv::print();

	log::init(argv::log_file, argv::log_level);

	log::cleanup();
	return 0;
}
