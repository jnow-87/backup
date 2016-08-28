bin-y := backup

backup-y := main.o argv.o opt.hash.o shell.o ui.o backup.o restore.o common/ cfg/


## additional dependencies for generated headers
$(loc_build_tree)/main.o: $(build_tree)/cfg/lexer.lex.h $(build_tree)/cfg/parser.tab.h
$(loc_build_tree)/argv.o: $(build_tree)/main/opt.hash.h
