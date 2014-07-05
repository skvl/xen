XEN_ROOT = $(CURDIR)/../../..
include $(XEN_ROOT)/tools/libfsimage/Rules.mk

CFLAGS += -DFSDIR="\"$(LIBEXEC_LIB)/fs\""

LDFLAGS-$(CONFIG_SunOS) = -Wl,-M -Wl,mapfile-SunOS
LDFLAGS-$(CONFIG_Linux) = -Wl,mapfile-GNU
LDFLAGS = $(LDFLAGS-y)

CFLAGS += $(PTHREAD_CFLAGS)
LDFLAGS += $(PTHREAD_LDFLAGS)

LIB_SRCS-y = fsimage.c fsimage_plugin.c fsimage_grub.c

PIC_OBJS := $(patsubst %.c,%.opic,$(LIB_SRCS-y))

LIB = libfsimage.so

.PHONY: all
all: $(LIB)

.PHONY: install
install: all
	$(INSTALL_DIR) $(DESTDIR)$(LIBEXEC_LIB)
	$(INSTALL_DIR) $(DESTDIR)$(includedir)
	$(INSTALL_PROG) libfsimage.so $(DESTDIR)$(LIBEXEC_LIB)
	$(INSTALL_DATA) fsimage.h $(DESTDIR)$(includedir)
	$(INSTALL_DATA) fsimage_plugin.h $(DESTDIR)$(includedir)
	$(INSTALL_DATA) fsimage_grub.h $(DESTDIR)$(includedir)

clean distclean::
	rm -f $(LIB)

libfsimage.so: $(PIC_OBJS)
	$(CC) $(LDFLAGS) -Wl,$(SONAME_LDFLAG),$@ $(SHLIB_LDFLAGS) -o $@ $^ $(PTHREAD_LIBS) $(APPEND_LDFLAGS)

-include $(DEPS)

