XEN_ROOT := $(CURDIR)/../../../
include $(XEN_ROOT)/tools/Rules.mk

LIBNAME            = libblktapctl

IBIN               = tap-ctl

CFLAGS            += -Werror
CFLAGS            += -Wno-unused
CFLAGS            += -I../include -I../drivers
CFLAGS            += $(CFLAGS_xeninclude)
CFLAGS            += $(CFLAGS_libxenctrl)
CFLAGS            += -D_GNU_SOURCE
CFLAGS            += -DTAPCTL

CTL_OBJS  := tap-ctl-ipc.o
CTL_OBJS  += tap-ctl-list.o
CTL_OBJS  += tap-ctl-allocate.o
CTL_OBJS  += tap-ctl-free.o
CTL_OBJS  += tap-ctl-create.o
CTL_OBJS  += tap-ctl-destroy.o
CTL_OBJS  += tap-ctl-spawn.o
CTL_OBJS  += tap-ctl-attach.o
CTL_OBJS  += tap-ctl-detach.o
CTL_OBJS  += tap-ctl-open.o
CTL_OBJS  += tap-ctl-close.o
CTL_OBJS  += tap-ctl-pause.o
CTL_OBJS  += tap-ctl-unpause.o
CTL_OBJS  += tap-ctl-major.o
CTL_OBJS  += tap-ctl-check.o

CTL_PICS  = $(patsubst %.o,%.opic,$(CTL_OBJS))

OBJS = $(CTL_OBJS) tap-ctl.o
PICS = $(CTL_PICS)

LIB_STATIC = $(LIBNAME).a
LIB_SHARED = $(LIBNAME).so
IBIN = tap-ctl

all: build

build: $(IBIN) $(LIB_STATIC) $(LIB_SHARED)

tap-ctl: tap-ctl.o $(LIBNAME).so
	$(CC) $(LDFLAGS) $(call LDFLAGS_RPATH,../lib) -o $@ $^

$(LIB_STATIC): $(CTL_OBJS)
	$(AR) r $@ $^

$(LIB_SHARED): $(CTL_PICS)
	$(CC) $(LDFLAGS) -fPIC $(SHLIB_LDFLAGS) -rdynamic $^ -o $@

install: $(IBIN) $(LIB_STATIC) $(LIB_SHARED)
	$(INSTALL_DIR) -p $(DESTDIR)$(IBDIR)
	$(INSTALL_DIR) -p $(DESTDIR)$(PRIVATE_LIBDIR)
	$(INSTALL_DIR) -p $(DESTDIR)$(PRIVATE_SBINDIR)
	$(INSTALL_PROG) $(IBIN) $(DESTDIR)$(PRIVATE_BINDIR)
	$(INSTALL_DATA) $(LIB_STATIC) $(DESTDIR)$(LIBDIR)
	$(INSTALL_PROG) $(LIB_SHARED) $(DESTDIR)$(PRIVATE_LIBDIR)

clean:
	rm -f $(OBJS) $(PICS) $(DEPS) $(IBIN) $(LIB_STATIC) $(LIB_SHARED)
	rm -f *~

.PHONY: all build clean install

-include $(DEPS)
