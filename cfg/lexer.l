%option prefix="cfg"
%option noyywrap

%{
	#include <common/log.h>
	#include <cfg/parser.tab.h>


	/* define YY_USER_ACTION, executed for every token, before excuting its
	 * action code
	 */
	#define YY_USER_ACTION { \
		/* update location based on current token */ \
		cfglloc.first_line = cfglloc.last_line = cfg_line; \
		cfglloc.first_column = cfg_col; \
		cfglloc.last_column = cfg_col + cfgleng; \
		cfg_col += cfgleng; \
	}

	/* make yyunput available outside of the lexer */
	static void yyunput(int c, char *buf_ptr);

	void cfgunput(char c){
		yyunput(c, (yytext_ptr));
	}
%}

/* start states */
%x MULTI_LINE_COMMENT


%%


%{
	/* code that goes to the beginning of yylex() */
	static unsigned int cfg_col = 1;
	static unsigned int cfg_line = 1;
%}

	/* specifier */
"config"						{ return SPEC_CFG; }
"dir"							{ return SPEC_DIR; }
"var"							{ return SPEC_VAR; }

	/* commont options */
".rsync-dir"					{ return CM_RSYNC_DIR; }

	/* config options */
".out-dir"						{ return CFG_OUT_DIR; }
".tmp-dir"						{ return CFG_TMP_DIR; }
".log-file"						{ return CFG_LOG_FILE; }
".archive"						{ return CFG_ARCHIVE; }
".indicate"						{ return CFG_INDICATE; }
".preserve"						{ return CFG_PRESERVE; }
".verbose"						{ return CFG_VERBOSE; }

	/* directory options */
".file"							{ return DIR_FILE; }

	/* file options */
".name"							{ return FILE_NAME; }

	/* comments */
\/\/.*\r?\n						{ cfg_col = 1; ++cfg_line; }
\/\*							{ BEGIN MULTI_LINE_COMMENT; }
<MULTI_LINE_COMMENT>\*\/		{ BEGIN INITIAL; }
<MULTI_LINE_COMMENT>.			{ }
<MULTI_LINE_COMMENT>\r?\n		{ cfg_col = 1; ++cfg_line; }

	/* identifier */
[a-zA-Z_][a-zA-Z_\-0-9]*		{ cfglval.c_str.s = cfgtext; cfglval.c_str.len = cfgleng; return IDFR; }

	/* string */
\"[a-zA-Z0-9._/\-\.\* ]+\"		{ cfglval.c_str.s = cfgtext + 1; cfglval.c_str.len = cfgleng - 2; return STRING; }

	/* integer */
[0-9]*							{ cfglval.i = atoi(cfgtext); return INT; }

	/* general patterns */
\r?\n							{ cfg_col = 1; ++cfg_line; }
[ \t]							{ }

	/* default */
.								{ return cfgtext[0]; }
