#ifndef SHELL_H
#define SHELL_H


/* types */
typedef enum{
	CMD_COPY = 1,
	CMD_RSYNC,
	CMD_MOVE
} cp_cmd_t;


/* prototypes */
int copy(char *src_dir, char *src_file, char *dst_base, char *dst_dir, cp_cmd_t cmd, bool indicate);
int unlink(char *dir, char *file, bool indicate);
int rmdir(char *dir, bool indicate);
int mkdir(char *base, char *dir, bool indicate);

bool yesno(char *text);


#endif // SHELL_H
