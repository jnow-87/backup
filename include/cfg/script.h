#ifndef SCRIPT_H
#define SCRIPT_H


/* class */
class script_t{
public:
	script_t();
	~script_t();

	char *cmd;

	script_t *prev,
			 *next;
};


/* prototypes */
void script_exec(script_t *lst, char const *info, bool indicate);


#endif // SCRIPT_H
