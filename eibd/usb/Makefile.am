EXTRA_DIST=README COPYING

if LINUX_API
TARGET = linux.c linux.h
else
TARGET =
endif

if HAVE_USB
LIBUSB = async.c devices.c descriptors.c list.c list.h libusb.h usb.c usbi.h $(TARGET)
PROG = findknxusb
else
LIBUSB =
PROB =
endif

bin_PROGRAMS= $(PROG)

findknxusb_SOURCES= findknxusb.cpp
findknxusb_LDADD= libusb.a

noinst_LIBRARIES=libusb.a
libusb_a_SOURCES=$(LIBUSB)
