XEN_ROOT=$(CURDIR)/../..
include $(XEN_ROOT)/tools/Rules.mk

CFLAGS += -Werror
CFLAGS += $(CFLAGS_libxenstore)

LDLIBS += $(LDLIBS_libxenstore)

.PHONY: all
all: xenpmd

.PHONY: install
install: all
	$(INSTALL_DIR) $(DESTDIR)$(LIBEXEC_BIN)
	$(INSTALL_PROG) xenpmd $(DESTDIR)$(LIBEXEC_BIN)

.PHONY: clean
clean:
	$(RM) -f xenpmd xenpmd.o $(DEPS_RM)

.PHONY: distclean
distclean: clean

.PHONY: uninstall
uninstall:
	rm -f $(DESTDIR)$(LIBEXEC_BIN)/xenpmd

xenpmd: xenpmd.o Makefile
	$(CC) $(LDFLAGS) $< -o $@ $(LDLIBS) $(APPEND_LDFLAGS)

-include $(DEPS_INCLUDE)
