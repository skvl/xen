# Copyright (C) HP Labs, Palo Alto and Fort Collins, 2005
# Author: Diwaker Gupta <diwaker.gupta@hp.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; under version 2 of the License.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

XEN_ROOT=$(CURDIR)/../..
include $(XEN_ROOT)/tools/Rules.mk

DEFAULT_PYTHON_PATH := $(shell $(XEN_ROOT)/tools/python/get-path)
PYTHON_PATH ?= $(DEFAULT_PYTHON_PATH)
INSTALL_PYTHON_PROG = $(XEN_ROOT)/tools/python/install-wrap "$(PYTHON_PATH)" $(INSTALL_PROG)

CFLAGS  += -Werror
CFLAGS  += $(CFLAGS_libxenctrl)
LDLIBS  += $(LDLIBS_libxenctrl)
LDFLAGS += $(call LDFLAGS_RPATH,../lib)

SCRIPTS = xenmon.py

.PHONY: all
all: build

.PHONY: build
build: xentrace_setmask xenbaked

.PHONY: install
install: build
	$(INSTALL_DIR) $(DESTDIR)$(PRIVATE_BINDIR)
	$(INSTALL_PROG) xenbaked $(DESTDIR)$(PRIVATE_BINDIR)/xenbaked
	$(INSTALL_PROG) xentrace_setmask  $(DESTDIR)$(PRIVATE_BINDIR)/xentrace_setmask
	$(INSTALL_PYTHON_PROG) xenmon.py  $(DESTDIR)$(PRIVATE_BINDIR)/xenmon
	$(INSTALL_DIR) $(DESTDIR)$(DOCDIR)
	$(INSTALL_DATA) README $(DESTDIR)$(DOCDIR)/README.xenmon

.PHONY: clean
clean:
	$(RM) -f $(DEPS)
	$(RM) -f xenbaked xenbaked.o
	$(RM) -f xentrace_setmask setmask.o

xenbaked: xenbaked.o Makefile
	$(CC) $(LDFLAGS) $< -o $@ $(LDLIBS) $(APPEND_LDFLAGS)

xentrace_setmask: setmask.o Makefile
	$(CC) $(LDFLAGS) $< -o $@ $(LDLIBS) $(APPEND_LDFLAGS)

-include $(DEPS)
