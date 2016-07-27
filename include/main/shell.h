#ifndef SHELL_H
#define SHELL_H


/* types */
typedef enum{
	CMD_COPY = 1,
	CMD_RSYNC,
	CMD_MOVE
} cp_cmd_t;


/* prototypes */

bool yesno(char *text);
int copy(char const *src_dir, char const *src_file, char const *dst_base, char const *dst_dir, cp_cmd_t cmd, bool indicate);
int unlink(char const *dir, char const *file, bool indicate);
int rmdir(char const *dir, bool indicate);
int mkdir(char const *base, char const *dir, bool indicate);


#endif // SHELL_H
