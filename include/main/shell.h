#ifndef SHELL_H
#define SHELL_H


/* macros */
#define SHELL(cmd, ...) \
	pshell(cmd, ##__VA_ARGS__)

#define SYSTEM(cmd, ...) \
	sshell(cmd " 2>&1", ##__VA_ARGS__)


/* types */
typedef enum{
	CMD_COPY = 1,
	CMD_RSYNC,
	CMD_MOVE
} cp_cmd_t;

typedef enum{
	FTYPE_ERR_FILE = -3,
	FTYPE_ERR_DIR = -2,
	FTYPE_ERR_BASE = -1,
	FTYPE_FILE = 1,
	FTYPE_DIR,
	FTYPE_OTHER,
} ftype_t;


/* external variables */
extern char shellerrstr[];


/* prototypes */
int copy(char const *sbase, char const *sdir, char const *sfile, char const *dbase, char const *ddir, char const *dfile, cp_cmd_t cmd, bool indicate);
int unlink(char const *dir, char const *file, bool indicate);
int rmdir(char const *dir, bool indicate);
int mkdir(char const *base, char const *dir, bool indicate);
int tar(char const *mode, char const *archive, char const *dir, char const *opt, bool indicate);
int diff(char const *sbase, char const *sdir, char const *sfile, char const *dbase, char const *ddir, char const *dfile);

ftype_t ftype(char const *base, char const *dir, char const *file);
int file_write(char const *file, char const *flags, char const *fmt, ...);

int pshell(char const *cmd, ...);
int sshell(char const *cmd, ...);


#endif // SHELL_H
