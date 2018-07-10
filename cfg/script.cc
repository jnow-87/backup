#include <common/log.h>
#include <common/list.h>
#include <cfg/script.h>
#include <main/ui.h>
#include <main/shell.h>


script_t::script_t(){
	cmd = 0;
}

script_t::~script_t(){
	delete cmd;
}

void script_exec(script_t *lst, char const *info){
	script_t *sit;


	USERHEAD("[executing %s scripts]", info);

	list_for_each(lst, sit){
		USER("%s ", sit->cmd);

		if(SHELL(sit->cmd) != 0)	USERERR("%s", shellerrstr);
		else						USEROK();
	}
}
