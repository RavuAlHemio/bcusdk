EXTRA_DIST=README COPYING

EIBD_USB = usb.cpp usb.h

if LINUX_API
AM_CFLAGS = -DTHREADS_POSIX
TARGET = linux_usbfs.c linux_usbfs.h
else
TARGET =
endif

if HAVE_USB
LIBUSB = $(EIBD_USB) libusb.h libusbi.h core.c descriptor.c io.c sync.c os/threads_posix.h os/poll_posix.h $(TARGET)
PROG = findknxusb
else
LIBUSB =
PROB =
endif

bin_PROGRAMS= $(PROG)

findknxusb_SOURCES= findknxusb.cpp
findknxusb_LDADD= libusb.a $(PTHSEM_LIBS)

noinst_LIBRARIES=libusb.a
libusb_a_SOURCES=$(LIBUSB) dummy.cpp
AM_CPPFLAGS=-I$(top_srcdir)/eibd/libserver -I$(top_srcdir)/common $(PTHSEM_CFLAGS)
