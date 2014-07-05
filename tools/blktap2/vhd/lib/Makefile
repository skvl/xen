XEN_ROOT=$(CURDIR)/../../../..
BLKTAP_ROOT := ../..
include $(XEN_ROOT)/tools/Rules.mk

LVM-UTIL-OBJ    := $(BLKTAP_ROOT)/lvm/lvm-util.o

CFLAGS          += -Werror
CFLAGS          += -Wno-unused
CFLAGS          += -I../../include
CFLAGS          += -D_GNU_SOURCE
CFLAGS          += -fPIC
CFLAGS		+= $(CFLAGS_libxenctrl)

ifeq ($(CONFIG_Linux),y)
LIBS            := -luuid
endif
LDFLAGS += $(LDFLAGS_libxenctrl) $(call LDFLAGS_RPATH)

ifeq ($(CONFIG_LIBICONV),y)
LIBS            += -liconv
endif

LIB-SRCS        := libvhd.c
LIB-SRCS        += libvhd-journal.c
LIB-SRCS        += vhd-util-coalesce.c
LIB-SRCS        += vhd-util-create.c
LIB-SRCS        += vhd-util-fill.c
LIB-SRCS        += vhd-util-modify.c
LIB-SRCS        += vhd-util-query.c
LIB-SRCS        += vhd-util-read.c
LIB-SRCS        += vhd-util-repair.c
LIB-SRCS        += vhd-util-resize.c
LIB-SRCS        += vhd-util-revert.c
LIB-SRCS        += vhd-util-set-field.c
LIB-SRCS        += vhd-util-snapshot.c
LIB-SRCS        += vhd-util-scan.c
LIB-SRCS        += vhd-util-check.c
LIB-SRCS        += vhd-util-uuid.c
LIB-SRCS        += relative-path.c
LIB-SRCS        += atomicio.c

LIB-OBJS         = $(patsubst %.c,%.o,$(LIB-SRCS))
LIB-OBJS        += $(LVM-UTIL-OBJ)

LIB-PICOBJS      = $(patsubst %.o,%.opic,$(LIB-OBJS))

LIBVHD           = libvhd.a libvhd.so

all: build

build: libvhd.a libvhd.so

libvhd.a: $(LIB-OBJS)
	$(AR) rc $@ $^

libvhd.so: $(LIB-PICOBJS)
	$(CC) $(SHLIB_LDFLAGS) $(LDFLAGS) -o libvhd.so $^ $(LIBS)

install: all
	$(INSTALL_DIR) -p $(DESTDIR)$(libdir)
	$(INSTALL_DATA) libvhd.a $(DESTDIR)$(libdir)
	$(INSTALL_PROG) libvhd.so $(DESTDIR)$(libdir)

clean:
	rm -rf *.a *.so* *.o *.opic *~ $(DEPS) $(LIBVHD)

distclean: clean

.PHONY: all build clean distclean install libvhd

-include $(DEPS)
