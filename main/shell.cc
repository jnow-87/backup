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
	char *dst;
	ftype_t sftype;


	/* check arguments */
	// init pointer
	dst = 0;

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

	// check src path
	sftype = ftype(sbase, sdir, sfile);

	switch(sftype){
	case FTYPE_ERR_BASE:
		USERERR("%s: %s", sbase, strerror(errno));
		return -1;

	case FTYPE_ERR_DIR:
		USERERR("%s%s: %s", sbase, sdir, strerror(errno));
		return -1;

	case FTYPE_ERR_FILE:
		USERERR("%s%s%s: %s", sbase, sdir, sfile, strerror(errno));
		return -1;

	default:
		break;
	}

	/* perform copy */
	// return if only indicating action
	if(indicate){
		USERINDICATE();
		return 0;
	}

	// create destination directory
	dst = dirname(dbase, ddir, dfile);

	if(dst == 0 || SHELL("mkdir -p %s", dst) != 0)
		goto err;

	// issue copy
	if(sftype == FTYPE_DIR){
		// copy directory, copy to dst to avoid creating a false directory,
		// e.g. cp /src/dir /dst/dir would create /dst/dir/dir
		if(SHELL("%s %s%s%s %s", cmd_str[cmd], sbase, sdir, sfile, dst) != 0)
			goto err;
	}
	else{
		// copy file
		if(SHELL("%s %s%s%s %s%s%s", cmd_str[cmd], sbase, sdir, sfile, dbase, ddir, dfile) != 0)
			goto err;
	}

	USEROK();
	delete dst;

	return 0;

err:
	USERERR("%s", errstr);
	delete dst;

	return -1;
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

ftype_t ftype(char const *base, char const *dir, char const *file){
	int fd_base,
		fd_dir;
	struct stat fs;


	base = STRNULL(base);
	dir = STRNULL(dir);
	file = STRNULL(file);

	// open base directory
	if(base[0] != 0){
		fd_base = open(base, O_RDONLY);

		if(fd_base == -1)
			return FTYPE_ERR_BASE;
	}

	// open directory
	if(base[0] == 0){
		// no base directory supplied
		if(dir[0] == 0)		fd_dir = open("./", O_RDONLY);
		else				fd_dir = open(dir, O_RDONLY);

		if(fd_dir == -1)
			return FTYPE_ERR_DIR;
	}
	else{
		// open within base directory
		if(dir[0] == 0)		fd_dir = openat(fd_base, "./", O_RDONLY);
		else				fd_dir = openat(fd_base, dir, O_RDONLY);

		close(fd_base);

		if(fd_dir == -1)
			return FTYPE_ERR_DIR;
	}

	// open file
	if(file[0] != 0 && file[strlen(file) - 1] == '*'){
		close(fd_dir);
		return FTYPE_OTHER;
	}

	if(fstatat(fd_dir, file, &fs, 0) != 0){
		close(fd_dir);
		return FTYPE_ERR_FILE;
	}

	close(fd_dir);

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

int diff(char const *sbase, char const *sdir, char const *sfile, char const *dbase, char const *ddir, char const *dfile){
	char const *difftool;
	int r;


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
	USER("diff %s%s%s against %s%s%s ", sbase, sdir, sfile, dbase, ddir, dfile);

	/* perform diff */
	switch(ftype(sbase, sdir, sfile)){
	case FTYPE_FILE:
	case FTYPE_OTHER:
		difftool = CONFIG_DIFF_TOOL_FILE;
		break;

	case FTYPE_DIR:
		difftool = CONFIG_DIFF_TOOL_DIR;
		break;

	default:
		USERERR("%s%s%s: %s", sbase, sdir, sfile, strerror(errno));
		return -1;
	}

	r = SHELL("%s \"%s %s%s%s %s%s%s ; echo -e '\n" FG_GREEN "done" RESET_ATTR " <press any key to continue>' ; read -sn1 x\"",
			CONFIG_TERMINAL, difftool,
			sbase, sdir, sfile,
			dbase, ddir, dfile
	);

	if(r != 0){
		USERERR("%s", errstr);
		return -1;
	}

	USEROK();

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
