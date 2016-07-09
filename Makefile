################
###   init   ###
################

# init build system variables
project_type := cxx
scripts_dir := scripts/build
config := .config
config_tree := scripts/config
use_config_sys := y
config_ftype := Pconfig
githooks_tree := .githooks
tool_deps :=

# include config
-include $(config)

# init source and build tree
default_build_tree := build/$(CONFIG_BUILD_TYPE)/
src_dirs := main/

# include build system Makefile
include $(scripts_dir)/Makefile.inc

# init default flags
cflags := $(CFLAGS) $(CONFIG_CFLAGS) -Wall -O2
cxxflags := $(CXXFLAGS) $(CONFIG_CXXFLAGS) -std=c++11 -Wall -O2
cppflags := $(CPPFLAGS) $(CONFIG_CPPFLAGS) -I"include/" -I"$(build_tree)/"
ldflags := $(LDFLAGS) $(CONFIG_LDFLAGS)
ldrflags := $(LDRFLAGS) $(CONFIG_LDRFLAGS)
asflags := $(ASFLAGS) $(CONFIG_ASFLAGS)
archflags := $(ARCHFLAGS) $(CONFIG_ARCHFLAGS)

yaccflags := $(YACCFLAGS) $(CONFIG_YACCFLAGS)
lexflags := $(LEXFLAGS) $(CONFIG_LEXFLAGS)
gperfflags := $(GPERFFLAGS) $(CONFIG_GPERFFLAGS)

###################
###   targets   ###
###################

####
## build
####
.PHONY: all
ifeq ($(CONFIG_BUILD_DEBUG),y)
all: cflags += -g -O0
all: cxxflags += -g -O0
all: asflags += -g -O0
endif

all: $(lib) $(bin)

####
## cleanup
####
.PHONY: clean
clean:
	$(rm) $(filter-out $(build_tree)/scripts,$(wildcard $(build_tree)/*))

.PHONY: distclean
distclean:
	$(rm) $(config) $(config).old .clang $(build_tree)

####
## install
####
.PHONY: install-user
install-user: all
	$(mkdir) -p ~/bin
	$(cp) -au $(build_tree)/main/backup ~/bin/

.PHONY: install-system
install-system: all
	$(mkdir) -p /usr/bin
	$(cp) -au $(build_tree)/main/backup /usr/bin/

.PHONY: uninstall
uninstall:
	$(rm) -rf /usr/bin/backup
	$(rm) -rf ~/bin/backup

