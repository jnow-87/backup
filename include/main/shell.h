#ifndef SHELL_H
#define SHELL_H


/* types */
typedef enum{
	CMD_COPY = 1,
	CMD_RSYNC,
	CMD_MOVE
} cp_cmd_t;

typedef enum{
	FTYPE_ERROR = -1,
	FTYPE_FILE = 1,
	FTYPE_DIR,
	FTYPE_OTHER,
} ftype_t;


/* prototypes */
int copy(char const *sbase, char const *sdir, char const *sfile, char const *dbase, char const *ddir, char const *dfile, cp_cmd_t cmd, bool indicate);
int unlink(char const *dir, char const *file, bool indicate);
int rmdir(char const *dir, bool indicate);
int mkdir(char const *base, char const *dir, bool indicate);
int tar(char const *mode, char const *archive, char const *dir, char const *opt, bool indicate);
int diff(char const *sbase, char const *sdir, char const *sfile, char const *dbase, char const *ddir, char const *dfile);

ftype_t ftype(char const *path, char const *file);
int file_write(char const *file, char const *flags, char const *fmt, ...);


#endif // SHELL_H
