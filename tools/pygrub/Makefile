
XEN_ROOT = $(CURDIR)/../..
include $(XEN_ROOT)/tools/Rules.mk

.PHONY: all
all: build
.PHONY: build
build:
	CC="$(CC)" CFLAGS="$(CFLAGS) $(APPEND_LDFLAGS)" $(PYTHON) setup.py build

.PHONY: install
install: all
	CC="$(CC)" CFLAGS="$(CFLAGS) $(APPEND_LDFLAGS)" $(PYTHON) setup.py install \
		$(PYTHON_PREFIX_ARG) --root="$(DESTDIR)" \
		--install-scripts=$(LIBEXEC_BIN) --force

.PHONY: clean
clean:
	rm -rf build tmp *.pyc *.pyo *.o *.a *~ a.out $(DEPS)

-include $(DEPS)
