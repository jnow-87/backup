#include <common/string.h>
#include <main/shell.h>
#include <main/ui.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <string>


using namespace std;


/* macros */
#define SHELL(cmd, ...) \
	shell(cmd " 2>&1", ##__VA_ARGS__)

#define ERRSTR(fmt, ...) \
	snprintf(errstr, MAXLEN, fmt, ##__VA_ARGS__)


/* static variables */
static char const *cmd_str[] = {
	"",
	"cp " CONFIG_CP_ARGS,
	"rsync " CONFIG_RSYNC_ARGS,
	"mv " CONFIG_MV_ARGS
};

static char const *cmd_txt[] = {
	"",
	"copy",
	"rsync",
	"move"
};

static char errstr[MAXLEN];


/* local/static prototypes */
int shell(char const *cmd, ...);


/* global functions */
/**
 * \brief	call shell copy/move command
 *
 * \param	sbase		source base (relative | absolute)
 * \param	sdir		source directory (relative | absolute)
 * \param	sfile		source file (relative)
 * \param	dbase		destination base (relative | absolute)
 * \param	ddir		destination directory (relative | absolute)
 * \param	dfile		destination file (relative)
 * \param	cmd			command to be called (copy, move, rsync)
 * \param	indicate	only print what would be execute, without actually executing the shell commands
 *
 * \return	0	success
 * 			-1	error
 */
int copy(char const *sbase, char const *sdir, char const *sfile, char const *dbase, char const *ddir, char const *dfile, cp_cmd_t cmd, bool indicate){
	int fd_base,
		fd_dir;
	struct stat fs;


	/* check arguments */
	// init pointer
	sbase = STRNULL(sbase);
	sdir = STRNULL(sdir);
	sfile = STRNULL(sfile);
	dbase = STRNULL(dbase);
	ddir = STRNULL(ddir);
	dfile = STRNULL(dfile);

	// avoid double '/'
	if(sbase[0] != 0 && sdir[0] == '/')
		++sdir;

	if(dbase[0] != 0 && ddir[0] == '/')
		++ddir;

	// user message
	USER("%s %s%s%s -> %s%s%s ", cmd_txt[cmd], sbase, sdir, sfile, dbase, ddir, dfile);

	// check src base
	if(sbase[0] != 0){
		fd_base = open(sbase, O_RDONLY);

		if(fd_base == -1){
			USERERR("%s: %s", sbase, strerror(errno));
			return -1;
		}
	}

	// check src directory
	if(sbase[0] == 0){
		// no base directory supplied
		if(sdir[0] == 0)	fd_dir = open("./", O_RDONLY);
		else				fd_dir = open(sdir, O_RDONLY);

		if(fd_dir == -1){
			USERERR("%s: %s", sdir, strerror(errno));
			return -1;
		}
	}
	else{
		// open within base directory
		fd_dir = openat(fd_base, sdir, O_RDONLY);

		close(fd_base);

		if(fd_dir == -1){
			USERERR("%s: %s", sdir, strerror(errno));
			return -1;
		}
	}

	// check src file, if it doesn't end on '*'
	if(strlen(sfile) == 0 || sfile[strlen(sfile) - 1] != '*'){
		if(fstatat(fd_dir, sfile, &fs, 0) != 0){
			USERERR("%s%s%s: %s", sbase, sdir, sfile, strerror(errno));
			close(fd_dir);
			return -1;
		}
	}

	close(fd_dir);

	/* perform copy */
	// return if only indicating action
	if(indicate){
		USERINDICATE();
		return 0;
	}

	// create target directory
	if(SHELL("mkdir -p %s%s", dbase, ddir) != 0){
		USERERR("%s", errstr);
		return -1;
	}

	// issue command
	if(SHELL("%s %s%s%s %s%s%s", cmd_str[cmd], sbase, sdir, sfile, dbase, ddir, dfile) != 0){
		USERERR("%s", errstr);
		return -1;
	}

	USEROK();

	return 0;
}

int unlink(char const *dir, char const *file, bool indicate){
	int fd,
		ret;


	ret = -1;
	/* check arguments */
	// init pointer
	dir = STRNULL(dir);
	file = STRNULL(file);

	// user message
	USER("remove %s%s ", dir, file);

	// directory
	fd = open(dir, O_RDONLY);

	if(fd == -1){
		USERERR("%s: %s", dir, strerror(errno));
		return -1;
	}

	/* performa unlink */
	if(indicate){
		USERINDICATE();
		ret = 0;

		goto clean;
	}

	if(unlinkat(fd, file, 0) != 0){
		USERERR("%s%s: %s", dir, file, strerror(errno));
		goto clean;
	}

	USEROK();
	ret = 0;

clean:
	close(fd);

	return ret;
}

int rmdir(char const *dir, bool indicate){
	/* check arguments */
	// init pointer
	dir = STRNULL(dir);

	// user message
	USER("remove %s ", STRNULL(dir));

	/* perform removal */
	if(indicate){
		USERINDICATE();
		return 0;
	}

	if(SHELL("rm -fr %s", dir) != 0){
		USERERR("%s", errstr);
		return -1;
	}

	USEROK();

	return 0;
}

int mkdir(char const *base, char const *dir, bool indicate){
	/* check arguments */
	// init pointer
	base = STRNULL(base);
	dir = STRNULL(dir);

	// avoid double '/'
	if(base[0] != 0 && dir[0] == '/')
		++dir;

	// user message
	USER("mkdir %s%s ", base, dir);

	/* perform mkdir */
	if(indicate){
		USERINDICATE();
		return 0;
	}

	if(SHELL("mkdir -p %s%s", base, dir) != 0){
		USERERR("%s", errstr);
		return -1;
	}

	USEROK();

	return 0;
}

int tar(char const *mode, char const *archive, char const *dir, char const *opt, bool indicate){
	/* check arguments */
	// init pointer
	mode = STRNULL(mode);
	archive = STRNULL(archive);
	dir = STRNULL(dir);
	opt = STRNULL(opt);

	// user message
	USER("tar %s %s -C %s %s ", mode, archive, dir, opt);

	/* perform tar */
	if(indicate){
		USERINDICATE();
		return 0;
	}

	if(SHELL("tar %s %s -C %s %s", mode, archive, dir, opt) != 0){
		USERERR("%s", errstr);
		return -1;
	}

	USEROK();

	return 0;
}

ftype_t ftype(char const *path, char const *file){
	int fd;
	struct stat fs;


	if(path == 0 || file == 0)
		return FTYPE_ERROR;

	// open path
	if(path[0] == 0)	fd = open("./", O_RDONLY);
	else				fd = open(path, O_RDONLY);

	if(fd == -1){
		ERROR("%s: %s\n", path, strerror(errno));
		return FTYPE_ERROR;
	}

	// get file stat
	if(fstatat(fd, file, &fs, 0) != 0){
		ERROR("%s%s: %s\n", path, file, strerror(errno));
		close(fd);
		return FTYPE_ERROR;
	}

	close(fd);

	if(fs.st_mode & S_IFDIR)	return FTYPE_DIR;
	if(fs.st_mode & S_IFREG)	return FTYPE_FILE;

	return FTYPE_OTHER;
}

int file_write(char const *file, char const *flags, char const *fmt, ...){
	FILE *fp;
	va_list lst;


	fp = fopen(file, flags);

	if(fp == 0)
		return -1;

	va_start(lst, fmt);
	vfprintf(fp, fmt, lst);
	va_end(lst);

	fclose(fp);

	return 0;
}


/* local functions */
int shell(char const *cmd, ...){
	FILE *fp;
	va_list lst;
	char buf[MAXLEN];
	int s;
	unsigned int i;


	/* issue cmd */
	va_start(lst, cmd);
	s = vsnprintf(buf, MAXLEN, cmd, lst);
	va_end(lst);

	if(s >= MAXLEN){
		ERRSTR("command exceeds maximum length of %d characters", MAXLEN);
		return -1;
	}

	LOGONLY("issue cmd: \"%s\"\n", buf);

	fp = popen(buf, "r");

	if(fp == 0)
		return -1;

	/* process cmd output */
	while(1){
		s = fread(buf, 1, MAXLEN - 1, fp);
		buf[s] = 0;

		USER2(buf);

		if(s != MAXLEN - 1)
			break;
	}

	/* close pipe */
	s = pclose(fp);

	if(WIFEXITED(s)){
		// strip '[\r\n]'
		i = 0;
		while(buf[i]){
			if(buf[i] == '\n' || buf[i] == '\r')
				buf[i] = ' ';
			++i;
		}

		ERRSTR("%s", buf);
		return -(WEXITSTATUS(s));
	}

	return -1;
}
