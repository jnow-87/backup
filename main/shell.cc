#include <main/shell.h>
#include <common/log.h>
#include <common/escape.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <string>


using namespace std;


/* macros */
#define MAXLEN	512

#define SHELL(cmd, ...) \
	shell(cmd " 2>&1", ##__VA_ARGS__)

#define USERIDICATE() \
	USER(FG_YELLOW "[INDICATE]" RESET_ATTR "\n");

#define USEROK() \
	USER(FG_GREEN "[OK]" RESET_ATTR "\n")

#define USERERR(msg, ...){ \
	USER(FG_RED "[ERR]"); \
	USER1(" (" msg ")", ##__VA_ARGS__); \
	USER(RESET_ATTR "\n"); \
}

#define ERRSTR(fmt, ...) \
	snprintf(errstr, MAXLEN, fmt, ##__VA_ARGS__)


/* static variables */
static char const *cmd_name[] = {
	"",
	"cp",
	"rsync",
	"mv"
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
void copy(char *src_dir, char *src_file, char *dst, cp_cmd_t cmd, bool indicate){
	int fd;
	struct stat fs;


	USER("%s %s/%s -> %s ", cmd_txt[cmd], src_dir, src_file, dst);

	/* check input */
	// check pointer
	if(src_dir == 0 || src_file == 0 || dst == 0){
		USERERR("null string argument");
		goto clean_0;
	}

	// check src directory
	fd = open(src_dir, O_RDONLY);

	if(fd == -1){
		USERERR("%s: %s", src_dir, strerror(errno));
		goto clean_1;
	}

	// check src file
	if(fstatat(fd, src_file, &fs, 0) != 0){
		USERERR("%s/%s: %s", src_dir, src_file, strerror(errno));
		goto clean_1;
	}

	/* perform copy */
	// return if only indicating action
	if(indicate){
		USERIDICATE();
		goto clean_1;
	}

	// create target directory
	if(mkdir(dst, 0755) != 0 && errno != EEXIST){
		USERERR("mkdir %s: %s", dst, strerror(errno));
		goto clean_1;
	}

	// issue command
	if(SHELL("%s %s/%s %s", cmd_name[cmd], src_dir, src_file, dst) != 0){
		USERERR("%s", errstr);
		goto clean_1;
	}

	USEROK();

	/* cleanup */
clean_1:
	close(fd);

clean_0:;
}

void unlink(char *dir, char *file){
	int fd;


	USER("remove %s/%s ", dir, file);

	if(dir == 0 || file == 0){
		USERERR("null string argument");
		return;
	}

	fd = open(dir, O_RDONLY);

	if(fd == -1){
		USERERR("%s: %s", dir, strerror(errno));
		return;
	}

	if(unlinkat(fd, file, 0) != 0){
		USERERR("%s/%s: %s", dir, file, strerror(errno));
		return;
	}

	close(fd);

	USEROK();
}

void rmdir(char *dir){
	USER("remove %s ", dir);

	if(dir == 0){
		USERERR("null string argument");
		return;
	}

	if(SHELL("rm -r %s", dir) != 0){
		USERERR("%s: %s", dir, strerror(errno));
		return;
	}

	USEROK();
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
