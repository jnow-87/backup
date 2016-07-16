#ifndef SHELL_H
#define SHELL_H


/* types */
typedef enum{
	CMD_COPY = 1,
	CMD_RSYNC,
	CMD_MOVE
} cp_cmd_t;


/* prototypes */
void copy(char *src_dir, char *src_file, char *dst, cp_cmd_t cmd, bool indicate);
void unlink(char *dir, char *file, bool indicate);
void rmdir(char *dir, bool indicate);

bool yesno(char *text);


#endif // SHELL_H
