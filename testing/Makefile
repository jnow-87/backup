# target calling all test cases
.PHONY: testing-all
testing-all: testing-backup testing-restore


####
## helper
####
FG := "\033[36m"
RESET_ATTR := "\033[0m"

define testmsg
	$(echo) $(FG) "\n\n\t ***" $(1) "***\n\n" $(RESET_ATTR)
endef

# global config
backup-bin := $(build_tree)/main/backup


##################
### test cases ###
##################

####
## backup
####

# test case comparing the output of the old backup shell script
# against the new implementation

# config
backup-sh := $(loc_src_tree)/backup
backup-sh-cfg := $(loc_src_tree)/backup.conf
backup-sh-out := /tmp/old-out
backup-sh-rsync := /tmp/old-rsync

backup-bin-cfg :=$(loc_src_tree)/backup.bc
backup-bin-out := /tmp/new-out
backup-bin-rsync := /tmp/new-rsync

# target
.PHONY: testing-backup
testing-backup:
	$(rm) $(backup-sh-out) $(backup-sh-rsync) $(backup-bin-out) $(backup-bin-rsync)
	$(call testmsg,"executing backup shell script")
	$(backup-sh) -c $(backup-sh-cfg)
	$(call testmsg,"executing backup binary")
	$(backup-bin) -f $(backup-bin-cfg)
	$(call testmsg,"untar shell script output")
	tar xzf $(backup-sh-out)/backup_* -C $(backup-sh-out)
	$(call testmsg,"diff output directories")
	diff -rq $(backup-sh-out) $(backup-bin-out); exit 0
	$(call testmsg,"diff rsync directories")
	diff -rq $(backup-sh-rsync) $(backup-bin-rsync); exit 0


####
## restore
####

# config
res-cfg := $(loc_src_tree)/restore.bc
res-chroot-arch := $(loc_src_tree)/chroot.tar.gz
res-out := /tmp/restore/out
res-chroot := /tmp/restore-chroot
res-chroot-cmp := /tmp/restore-chroot-cmp

.PHONY: testing-restore
testing-restore:
	$(rm) $(dir $(res-out)) $(res-chroot) $(res-chroot-cmp)
	$(mkdir) $(res-chroot) $(res-chroot-cmp)
	$(call testmsg,"untar chroot")
	tar xzf $(res-chroot-arch) -C $(res-chroot)
	tar xzf $(res-chroot-arch) -C $(res-chroot-cmp)
	$(call testmsg,"backup chroot")
	$(backup-bin) -f $(res-cfg)
	$(rm) $(res-chroot)
	$(call testmsg,"restore")
	$(backup-bin) -r $(res-out)
	$(call testmsg,"diff" $(res-chroot) "and" $(res-chroot-cmp))
	diff -rq $(res-chroot) $(res-chroot-cmp)
