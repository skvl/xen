XEN_ROOT = $(CURDIR)/../..
include $(XEN_ROOT)/tools/Rules.mk

SUBDIRS-y :=
SUBDIRS-y += libelf

#SUBDIRS-y += x86_instruction_emulator
# This does not compile with Debian's userland compile options,
# which include fortify etc.

.PHONY: all clean distclean install uninstall
all clean distclean install uninstall: %: subdirs-%
