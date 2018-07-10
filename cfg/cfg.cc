#include <cfg/cfg.h>


cfg_t::cfg_t(){
	name = 0;
	out_dir = 0;
	tmp_dir = 0;
	rsync_dir = 0;
	log_file = 0;

	archive= false;
	indicate = false;
	preserve = false;
	noconfig = false;
	nodata = false;

	verbosity = 0;

	pre_backup_lst = 0;
	post_backup_lst = 0;
	pre_restore_lst = 0;
	post_restore_lst = 0;
}

cfg_t::~cfg_t(){
	delete [] name;
	delete [] out_dir;
	delete [] tmp_dir;
	delete [] rsync_dir;
	delete [] log_file;
}
