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
#define PATHCONCAT(base, dir) \
	(((dir) != 0 && (dir)[0] == '/') ? "" : STRNULL(base)), STRNULL(dir)

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
int copy(char const *src_dir, char const *src_file, char const *dst_base, char const *dst_dir, char const *dst_file, cp_cmd_t cmd, bool indicate){
	int fd;
	struct stat fs;


	USER("%s %s%s -> %s%s%s ", cmd_txt[cmd], PATHCONCAT(src_dir, src_file), PATHCONCAT(dst_base, dst_dir), STRNULL(dst_file));

	/* check arguments */
	// pointer
	if(src_dir == 0 || src_file == 0 || dst_dir == 0 || (dst_dir[0] != '/' && dst_base == 0) || dst_file == 0){
		USERERR("null string argument");
		return -1;
	}

	// check src directory
	if(src_file[0] == '/')		fd = open("/", O_RDONLY);
	else if(src_dir[0] == 0)	fd = open("./", O_RDONLY);
	else						fd = open(src_dir, O_RDONLY);

	if(fd == -1){
		USERERR("%s: %s", src_dir, strerror(errno));
		return -1;
	}

	// check src file, if it doesn't end on '*'
	if(src_file[strlen(src_file) - 1] != '*'){
		if(fstatat(fd, src_file, &fs, 0) != 0){
			USERERR("%s%s: %s", PATHCONCAT(src_dir, src_file), strerror(errno));
			close(fd);
			return -1;
		}
	}

	close(fd);

	/* perform copy */
	// return if only indicating action
	if(indicate){
		USERINDICATE();
		return 0;
	}

	// create target directory
	if(SHELL("mkdir -p %s%s", PATHCONCAT(dst_base, dst_dir)) != 0){
		USERERR("%s", errstr);
		return -1;
	}

	// issue command
	if(SHELL("%s %s%s %s%s%s", cmd_str[cmd], PATHCONCAT(src_dir, src_file), PATHCONCAT(dst_base, dst_dir), dst_file) != 0){
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
	USER("remove %s%s ", STRNULL(dir), STRNULL(file));

	/* check arguments */
	// pointer
	if(dir == 0 || file == 0){
		USERERR("null string argument");
		return -1;
	}

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
	USER("remove %s ", STRNULL(dir));

	/* check arguments */
	if(dir == 0){
		USERERR("null string argument");
		return -1;
	}

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
	USER("mkdir %s%s ", PATHCONCAT(base, dir));

	/* check arguments */
	if(base == 0 && dir == 0){
		USERERR("null string argument");
		return -1;
	}

	/* perform mkdir */
	if(indicate){
		USERINDICATE();
		return 0;
	}

	if(SHELL("mkdir -p %s%s", PATHCONCAT(base, dir)) != 0){
		USERERR("%s", errstr);
		return -1;
	}

	USEROK();

	return 0;
}

int tar(char const *mode, char const *archive, char const *dir, char const *opt, bool indicate){
	USER("tar %s %s -C %s %s ", STRNULL(mode), STRNULL(archive), STRNULL(dir), STRNULL(opt));

	/* check arguments */
	if(mode == 0 || dir == 0 || archive == 0 || opt == 0){
		USERERR("null string argument");
		return -1;
	}

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
	if(file[0] == '/')		fd = open("/", O_RDONLY);
	else if(path[0] == 0)	fd = open("./", O_RDONLY);
	else					fd = open(path, O_RDONLY);

	if(fd == -1){
		ERROR("%s: %s\n", path, strerror(errno));
		return FTYPE_ERROR;
	}

	// get file stat
	if(fstatat(fd, file, &fs, 0) != 0){
		ERROR("%s%s: %s\n", PATHCONCAT(path, file), strerror(errno));
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
