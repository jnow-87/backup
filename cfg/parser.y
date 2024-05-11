%define api.prefix {cfg}
%define parse.error verbose
%define parse.lac full
%locations

/* header */
%{
	#include <common/log.h>
	#include <common/list.h>
	#include <common/string.h>
	#include <common/escape.h>
	#include <cfg/lexer.lex.h>
	#include <cfg/cfg.h>
	#include <cfg/dir.h>
	#include <main/argv.h>
	#include <string.h>
	#include <errno.h>
	#include <map>
	#include <string>


	/* macros */
	// parser error message
	#define PARSER_ERROR(s, ...){ \
		ERROR(FG_VIOLETT "%s" RESET_ATTR ":" FG_GREEN "%d:%d " RESET_ATTR s, file, cfglloc.first_line, cfglloc.first_column, ##__VA_ARGS__); \
		YYERROR; \
	}

	/**
	 * \brief	Check if the given map contains an entry with the
	 *			given name. Return the respective entry if present,
	 *			otherwise trigger a parser error
	 */
	#define VAR_MAP_LOOKUP(map, _name)({ \
		string name = _name; \
		auto it = (map).find(name); \
		\
		if(it == (map).end()) \
			PARSER_ERROR("variable \"%s\" not defined\n", (name).c_str()) \
		\
		it->second; \
	})


	/* local/static variables */
	static FILE *fp = 0;
	static map<string, int> vmap_int;
	static map<string, string*> vmap_str;


	/* prototypes */
	extern void cfgunput(char c);
	static int cfgerror(char *file, cfg_t **cfg_lst, dir_t **dir_lst, bool *valid, char const *s);
	static void scriptalloc(script_t **lst, string *cmd);
%}

%code requires{
	#include <cfg/cfg.h>
	#include <cfg/dir.h>
	#include <string>


	/* types */
	typedef enum{
		VAR_INT = 1,
		VAR_STR,
	} var_type_t;
}

/* parse paramters */
%parse-param { char *file }
%parse-param { cfg_t **cfg_lst }
%parse-param { dir_t **dir_lst }
%parse-param { bool *valid }

/* init code */
%initial-action{
	/* init results */
	*valid = true;
	*cfg_lst = 0;
	*dir_lst = 0;

	/* open config file */
	fp = fopen(file, "r");

	if(fp == 0){
		ERROR("error reading config file \"%s\" -- %s\n", file, strerror(errno));

		*valid = false;

		return 1;
	}

	/* start lexer */
	cfgrestart(fp);
}

/* cleanup code */
%destructor{
	/* cleanup lexer */
	cfglex_destroy();

	/* close config file */
	fclose(fp);

	/* cleanup variables lists */
	for(auto it=vmap_str.begin(); it!=vmap_str.end(); ++it)
		delete it->second;

	vmap_str.clear();
	vmap_int.clear();
} destructor

/* parser union type */
%union{
	int i;

	cfg_t *cfg;
	dir_t *dir;
	file_t *file;
	string *cpp_str;

	struct{
		char *s;
		unsigned int len;
	} c_str;

	struct{
		union{
			string *s;
			int i;
		};

		var_type_t type;
	} var;
}

/* terminals */
// general
%token <i> INT
%token <c_str> STRING
%token <c_str> IDFR

// specifier
%token SPEC_CFG
%token SPEC_DIR
%token SPEC_VAR

// common options
%token CM_RSYNC_DIR

// config options
%token CFG_OUT_DIR
%token CFG_TMP_DIR
%token CFG_LOG_FILE
%token CFG_ARCHIVE
%token CFG_INDICATE
%token CFG_PRESERVE
%token CFG_NOCONFIG
%token CFG_NODATA
%token CFG_VERBOSE
%token CFG_LOGFILE
%token CFG_PRE_BACKUP
%token CFG_POST_BACKUP
%token CFG_PRE_RESTORE
%token CFG_POST_RESTORE

// directory options
%token DIR_FILE

// file options
%token FILE_NAME

/* non-terminals */
%type <i> destructor
%type <cfg> config-list
%type <cfg> config-member
%type <dir> dir-list
%type <dir> dir-member
%type <file> file-member
%type <var> variable
%type <cpp_str> string
%type <i> integer


%%


/* start */
destructor :	start												{ $$ = 0; }
	  	   ;

start :	%empty														{ }
	  |	error ';'													{ *valid = false; }
	  |	start stmt ';' 												{ }
	  ;

/* statement */
stmt :	SPEC_CFG IDFR '=' '{' config-list '}'						{ $5->name = stralloc($2.s, $2.len, 0); list_add_tail(cfg_lst, $5); }
	 |	SPEC_DIR STRING '=' '{' dir-list '}'						{
	 																	$5->path = stralloc($2.s, $2.len, PF_DIR | PF_ABS);
																		list_add_tail(dir_lst, $5);

																		/* warning for path conversion */
	 																	if($2.s[0] != '/'){
																			WARN(FG_VIOLETT "%s" RESET_ATTR ":" FG_GREEN "%d:%d" RESET_ATTR " relative directory \"%*.*s\" converted to absolute path \"%s\"\n",
																				file, @2.first_line, @2.first_column,
																				$2.len, $2.len, $2.s,
																				$5->path
																			);
																		}
																	}
	 |	SPEC_VAR IDFR '=' variable									{
																		string name = string($2.s, $2.len);

																		/* update variable map */
	 																	if($4.type == VAR_STR){
																			// delete string if the variable already exists
																			if(vmap_str.find(name) != vmap_str.end())
																				delete vmap_str[name];

																			vmap_str[name] = $4.s;
																		}
																		else if($4.type == VAR_INT)
																			vmap_int[name] = $4.i;
																	}
	 ;

/* config */
config-list :	config-member										{ $$ = $1; }
			|	config-member ','									{ $$ = $1; }
			;

config-member :	%empty												{ $$ = new cfg_t(); cfgunput(','); }
			  |	config-member ',' CFG_OUT_DIR '=' string			{ $$->out_dir = stralloc((char*)($5->c_str()), $5->length(), PF_DIR); delete $5; }
			  |	config-member ',' CFG_TMP_DIR '=' string			{ $$->tmp_dir = stralloc((char*)($5->c_str()), $5->length(), PF_DIR); delete $5; }
			  |	config-member ',' CFG_LOG_FILE '=' string			{ $$->log_file = stralloc((char*)($5->c_str()), $5->length(), PF_FILE); delete $5; }
			  |	config-member ',' CM_RSYNC_DIR '=' string			{ $$->rsync_dir = stralloc((char*)($5->c_str()), $5->length(), PF_DIR); delete $5; }
			  |	config-member ',' CFG_ARCHIVE '=' integer			{ $$->archive = $5; }
			  |	config-member ',' CFG_INDICATE '=' integer			{ $$->indicate = $5; }
			  |	config-member ',' CFG_PRESERVE '=' integer			{ $$->preserve = $5; }
			  |	config-member ',' CFG_NOCONFIG '=' integer			{ $$->noconfig = $5; }
			  |	config-member ',' CFG_NODATA '=' integer			{ $$->nodata = $5; }
			  |	config-member ',' CFG_VERBOSE '=' integer			{ $$->verbosity = $5; }
			  | config-member ',' CFG_PRE_BACKUP '=' string			{ $$ = $1; scriptalloc(&($$->pre_backup_lst), $5); delete $5; }
			  | config-member ',' CFG_POST_BACKUP '=' string		{ $$ = $1; scriptalloc(&($$->post_backup_lst), $5); delete $5; }
			  | config-member ',' CFG_PRE_RESTORE '=' string		{ $$ = $1; scriptalloc(&($$->pre_restore_lst), $5); delete $5; }
			  | config-member ',' CFG_POST_RESTORE '=' string		{ $$ = $1; scriptalloc(&($$->post_restore_lst), $5); delete $5; }
			  ;

/* directory */
dir-list :	dir-member												{ $$ = $1; }
		 |	dir-member ','											{ $$ = $1; }
		 ;

dir-member :	%empty												{ $$ = new dir_t(); cfgunput(','); }
		   |	dir-member ',' DIR_FILE '=' '{' file-member '}'		{ $$ = $1; list_add_tail(&($$->file_lst), $6); }
		   ;

file-member :	%empty												{ $$ = new file_t(); cfgunput(','); }
			|	file-member ',' FILE_NAME '=' string				{
																		$$ = $1;
																		$$->name = stralloc((char*)($5->c_str()), $5->length(), PF_REL);

	 																	/* warning for path conversion */
																		if($5->c_str()[0] == '/'){
																			WARN(FG_VIOLETT "%s" RESET_ATTR ":" FG_GREEN "%d:%d" RESET_ATTR " absolute file name \"%*.*s\" converted to relative path \"%s\"\n",
																				file, @5.first_line, @5.first_column,
																				$5->length(), $5->length(), $5->c_str(),
																				$$->name
																			);
																		}

																		delete $5;
																	}
			|	file-member ',' CM_RSYNC_DIR '=' string				{
																		$$ = $1;
																		$$->rsync_dir = stralloc((char*)($5->c_str()), $5->length(), PF_DIR | PF_REL);

	 																	/* warning for path conversion */
																		if($5->c_str()[0] == '/'){
																			WARN(FG_VIOLETT "%s" RESET_ATTR ":" FG_GREEN "%d:%d" RESET_ATTR " absolute rsync directory \"%*.*s\" converted to relative path \"%s\"\n",
																				file, @5.first_line, @5.first_column,
																				$5->length(), $5->length(), $5->c_str(),
																				$$->rsync_dir
																			);
																		}

																		delete $5;
																	}
			;

/* variable */
variable :	STRING													{ $$.type = VAR_STR; $$.s = new string($1.s, $1.len); }
		 |	INT														{ $$.type = VAR_INT; $$.i = $1; }
		 |	IDFR													{
		 																string name($1.s, $1.len);

																		/* return value of variable with given identifier */
																		if(vmap_str.find(name) != vmap_str.end()){
																			$$.type = VAR_STR;
																			$$.s = new string(*vmap_str[name]);
																		}
																		else if(vmap_int.find(name) != vmap_int.end()){
																			$$.type = VAR_INT;
																			$$.i = vmap_int[name];
																		}
																		else
																			PARSER_ERROR("variable \"%s\" not defined\n", name.c_str())
		 															}
		 |	variable '+' STRING										{
		 																$$ = $1;

																		if($$.type != VAR_STR)
																			PARSER_ERROR("variable is no string\n");

																		$$.s->append($3.s, $3.len);
																	}
		 |	variable '+' INT										{
		 																$$ = $1;

																		if($$.type != VAR_INT)
																			PARSER_ERROR("variable is no integer\n");

																		$$.i += $3;
																	}
		 |	variable '+' IDFR										{
																		$$ = $1;

																		/* ensure that only matching types are assigned */
																		if($$.type == VAR_STR)		$$.s->append(*VAR_MAP_LOOKUP(vmap_str, string($3.s, $3.len)));
																		else if($$.type == VAR_INT)	$$.i += VAR_MAP_LOOKUP(vmap_int, string($3.s, $3.len));
																		else						PARSER_ERROR("variable \"%*.*s\" not defined\n", $3.len, $3.len, $3.s)
		 															}
		 ;

/* basic types */
string :	STRING													{ $$ = new string($1.s, $1.len); }
	   |	IDFR													{ $$ = new string(*VAR_MAP_LOOKUP(vmap_str, string($1.s, $1.len))); }
	   |	string '+' STRING										{ $$ = $1; $$->append($3.s, $3.len); }
	   |	string '+' IDFR											{ $$ = $1; $$->append(*VAR_MAP_LOOKUP(vmap_str, string($3.s, $3.len))); }
	   ;

integer :	INT														{ $$ = $1; }
		|	IDFR													{ $$ = VAR_MAP_LOOKUP(vmap_int, string($1.s, $1.len)); }
	    ;


%%


static int cfgerror(char *file, cfg_t **cfg_lst, dir_t **dir_lst, bool *valid, char const *s){
	ERROR(FG_VIOLETT "%s" RESET_ATTR ":" FG_GREEN "%d:%d" RESET_ATTR " token \"%s\" -- %s\n", file, cfglloc.first_line, cfglloc.first_column, cfgtext, s);
	return 0;
}

static void scriptalloc(script_t **lst, string *cmd){
	script_t *scr;


	scr = new script_t;
	scr->cmd = stralloc((char*)(cmd->c_str()), cmd->length(), PF_FILE);

	list_add_tail(lst, scr);
}
