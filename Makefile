#SHELL = /bin/sh
#INSTALL = /usr/bin/install
#INSTALL_PROGRAM = $(INSTALL)
#INSTALL_DATA = $(INSTALL) -m 644
#include Makefile.conf

# Compile all the applications or just the strategy example
# DIRS = apps/verifier apps/seqdacon apps/strategy_example_1 apps/price-hunter-hub
DIRS = apps/strategy_example_1

# the sets of directories to do various things in
BUILDDIRS = $(DIRS:%=build-%)
INSTALLDIRS = $(DIRS:%=install-%)
CLEANDIRS = $(DIRS:%=clean-%)
TESTDIRS = $(DIRS:%=test-%)

all: $(BUILDDIRS)
$(DIRS): $(BUILDDIRS)
$(BUILDDIRS):
	$(MAKE) -C $(@:build-%=%)

# the utils need the libraries in dev built first
#build-utils: build-dev

test: $(TESTDIRS) all
$(TESTDIRS):
	$(MAKE) -C $(@:test-%=%) test

clean: $(CLEANDIRS)
$(CLEANDIRS):
	$(MAKE) -C $(@:clean-%=%) clean


.PHONY: subdirs $(DIRS)
.PHONY: subdirs $(BUILDDIRS)
#.PHONY: subdirs $(INSTALLDIRS)
#.PHONY: subdirs $(TESTDIRS)
.PHONY: subdirs $(CLEANDIRS)
.PHONY: all install clean test

watch:
	while true; do (time make; inotifywait -qre close_write .; ) done;
