bin_PROGRAMS=bcuaddrtab
AM_CPPFLAGS=-I../../common -I../libserver -I../backend -I../usb
BACKEND_CONF= l-FT12.h l-PEI16.h l-PEI16s.h l-USB.h
bcuaddrtab_SOURCES=bcuaddrtab.cpp addrtab.cpp addrtab.h lowlevelcreate.h lowlevelconf.h $(BACKEND_CONF)
bcuaddrtab_LDADD=../backend/libbackend.a ../libserver/libeibstack.a ../../common/libcommon.a ../usb/libusb.a
