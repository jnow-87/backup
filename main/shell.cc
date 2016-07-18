#include <main/shell.h>
#include <main/ui.h>
#include <common/string.h>
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
#define MAXLEN	512

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
int copy(char *src_dir, char *src_file, char *dst_base, char *dst_dir, cp_cmd_t cmd, bool indicate){
	int fd;
	int ret;
	struct stat fs;


	ret = -1;
	USER("%s %s%s -> %s%s ", cmd_txt[cmd], STRNULL(src_dir), STRNULL(src_file), PATHCONCAT(dst_base, dst_dir));

	/* check arguments */
	// pointer
	if(src_dir == 0 || src_file == 0 || dst_dir == 0 || (dst_dir[0] != '/' && dst_base == 0)){
		USERERR("null string argument");
		return -1;
	}

	// check src directory
	fd = open(src_dir, O_RDONLY);

	if(fd == -1){
		USERERR("%s: %s", src_dir, strerror(errno));
		goto clean;
	}

	// check src file
	if(fstatat(fd, src_file, &fs, 0) != 0){
		USERERR("%s%s: %s", src_dir, src_file, strerror(errno));
		goto clean;
	}

	/* perform copy */
	// return if only indicating action
	if(indicate){
		USERINDICATE();
		ret = 0;

		goto clean;
	}

	// create target directory
	if(SHELL("mkdir -p %s%s", PATHCONCAT(dst_base, dst_dir)) != 0){
		USERERR("mkdir %s%s: %s", PATHCONCAT(dst_base, dst_dir), strerror(errno));
		goto clean;
	}

	// issue command
	if(SHELL("%s %s%s %s%s", cmd_str[cmd], src_dir, src_file, PATHCONCAT(dst_base, dst_dir)) != 0){
		USERERR("%s", errstr);
		goto clean;
	}

	USEROK();
	ret = 0;

	/* cleanup */
clean:
	close(fd);

	return ret;
}

int unlink(char *dir, char *file, bool indicate){
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

int rmdir(char *dir, bool indicate){
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
		USERERR("%s: %s", dir, errstr);
		return -1;
	}

	USEROK();

	return 0;
}

int mkdir(char *base, char *dir, bool indicate){
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
		USERERR("mkdir %s%s: %s", PATHCONCAT(base, dir), strerror(errno));
		return -1;
	}

	USEROK();

	return 0;
}

bool yesno(char *text){
	char c;


	if(text == 0)
		return false;

	USER("%s [y/N]\n", text);

	while(fread(&c, 1, 1, stdin) != 1)
		usleep(100000);

	if(c == 'y' || c == 'Y')
		return true;
	return false;
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

		ERRSTR("shell cmd failed: %s", buf);
		return -(WEXITSTATUS(s));
	}

	return -1;
}
