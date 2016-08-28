#include <common/log.h>
#include <common/list.h>
#include <common/string.h>
#include <cfg/cfg.h>
#include <cfg/dir.h>
#include <cfg/file.h>
#include <main/ui.h>
#include <main/shell.h>
#include <main/argv.h>


/**
 * TODO
 * 	- copy files
 * 		- ask for every file to
 * 			- copy/all		c/C
 * 			- move/all		m/M
 * 			- skip/all		s/S
 * 			- show diff/all	d/D
 */
void restore(cfg_t *cfg, dir_t *dir_lst){
	/* check user */
	if(!yesno("continue restore?"))
		return;

	USERHEAD("[restore from \"%s\"]", argv::archive);

	/* handle files w/o rsync directory */
	if(!cfg->noconfig){
	}

	/* handle files with rsync directory */
	if(!cfg->nodata){
	}
}
