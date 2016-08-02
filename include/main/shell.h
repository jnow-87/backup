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
int copy(char const *src_dir, char const *src_file, char const *dst_base, char const *dst_dir, char const *dst_file, cp_cmd_t cmd, bool indicate);
int unlink(char const *dir, char const *file, bool indicate);
int rmdir(char const *dir, bool indicate);
int mkdir(char const *base, char const *dir, bool indicate);
int tar(char const *mode, char const *archive, char const *dir, char const *opt, bool indicate);

ftype_t ftype(char const *path, char const *file);


#endif // SHELL_H
