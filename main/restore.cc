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
	static char c = 0;
	bool file_done;
	char const *sdir,
		 	   *sfile,
			   *dbase,
			   *ddir,
			   *dfile;


	/* init */
	file_done = false;

	// source
	sfile = file->name;
	sdir = dir->path;

	// destination
	dbase = "";
	ddir = dir->path;
	dfile = file->name;

	// handle files with rsync directories
	if(file->rsync_dir){
		sfile = filename(file->name);
		sdir = file->rsync_dir;

		// handle rsync special cases
		// 	dir/:	if the file name contains trailing '/' the content of the
		// 			original directory rather than the directory itself has
		// 			been copied to the backup, hence only the content needs
		//			to be restored
		//			to ensure that dfile is not truncated, it is not used,
		//			instead dbase and ddir are used
		if(STRLASTC(file->name) == '/'){
			sfile = "*";

			dbase = dir->path;
			ddir = file->name;
			dfile = "";
		}
	}

	/* get and execute user action */
	while(!file_done){
		// get user selection
		if(!(copy_all || move_all || diff_all || skip_all) || c == 0){
			c = uinput("%s%s: "
					   "copy/all " BOLD "[c/C]" RESET_ATTR ", "
					   "move/all " BOLD "[m/M]" RESET_ATTR ", "
					   "diff/all " BOLD "[d/D]" RESET_ATTR ", "
					   "ship/all " BOLD "[s/S]" RESET_ATTR "\n",
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
			copy(base, sdir, sfile, dbase, ddir, dfile, CMD_COPY, indicate);
			file_done = true;
			break;

		// move
		case 'M':
			move_all = true;
			// fall through

		case 'm':
			copy(base, sdir, sfile, dbase, ddir, dfile, CMD_MOVE, indicate);
			file_done = true;
			break;

		// diff
		case 'D':
			diff_all = true;
			// fall through

		case 'd':
			diff(base, sdir, sfile, dbase, ddir, dfile);
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
}
