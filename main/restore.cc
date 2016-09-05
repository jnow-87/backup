#include <common/log.h>
#include <common/list.h>
#include <common/string.h>
#include <cfg/cfg.h>
#include <cfg/dir.h>
#include <cfg/file.h>
#include <main/ui.h>
#include <main/shell.h>
#include <main/argv.h>


/* static variables */
static bool copy_all = false,
			move_all = false,
			diff_all = false,
			skip_all = false;


/* local/static prototypes */
void handle_file(char const *base, dir_t *dir, file_t *file, bool indicate);


/* global functions */
void restore(cfg_t *cfg, dir_t *dir_lst){
	dir_t *dir;
	file_t *file;


	USER("restore from \"%s\"", argv::archive);

	/* handle files w/o rsync directory */
	if(!cfg->noconfig && yesno("\nrestore config files?")){
		USERHEAD("[processing config files (files w/o rsync directory)");

		list_for_each(dir_lst, dir){
			list_for_each(dir->file_lst, file){
				if(file->rsync_dir == 0)
					handle_file(cfg->tmp_dir, dir, file, cfg->indicate);
			}
		}
	}

	/* reset global user input */
	copy_all = false;
	move_all = false;
	diff_all = false;
	skip_all = false;

	/* handle files with rsync directory */
	if(!cfg->nodata && yesno("\nrestore data files?")){
		USERHEAD("[processing data files (files with rsync directory)]");

		list_for_each(dir_lst, dir){
			list_for_each(dir->file_lst, file){
				if(file->rsync_dir != 0)
					handle_file(cfg->rsync_dir, dir, file, cfg->indicate);
			}
		}
	}
}


/* local functions */
void handle_file(char const *base, dir_t *dir, file_t *file, bool indicate){
	bool file_done;
	char c,
		 *dst,
		 *sdir;


	file_done = false;
	sdir = (file->rsync_dir ? file->rsync_dir : dir->path);

	// use dirname, avoid generating too many sub-directories when copying directories
	// e.g. cp -r /base/dir/ /tmp/base/dir creates the output directory /tmp/base/dir/dir
	dst = dirname(dir->path, file->name);

	while(!file_done){
		// get user selection
		if(!(copy_all || move_all || diff_all || skip_all)){
			c = uinput("%s%s: copy/all [c/C], move/all [m/M], diff/all [d/D], ship/all [s/S]\n",
					   "cCmMdDsS",
					   dir->path, file->name
			);
		}

		// handle user selection
		switch(c){
		// copy
		case 'C':
			copy_all = true;
			// fall through

		case 'c':
			copy(base, sdir, file->name, "", dst, "", CMD_COPY, indicate);
			file_done = true;
			break;

		// move
		case 'M':
			move_all = true;
			// fall through

		case 'm':
			copy(base, sdir, file->name, "", dst, "", CMD_MOVE, indicate);
			file_done = true;
			break;

		// diff
		case 'D':
			diff_all = true;
			// fall through

		case 'd':
			diff(base, sdir, file->name, "", dir->path, file->name);
			break;

		// skip
		case 'S':
			skip_all = true;
			// fall through

		case 's':
			file_done = true;
			break;
		}
	}

	delete dst;
}
