bin_PROGRAMS=bcuaddrtab
AM_CPPFLAGS=-I../../common -I../libserver -I../backend
BACKEND_CONF= l-FT12.h l-PEI16.h l-PEI16s.h
bcuaddrtab_SOURCES=bcuaddrtab.cpp addrtab.cpp addrtab.h lowlevelcreate.h lowlevelconf.h $(BACKEND_CONF)
bcuaddrtab_LDADD=../backend/libbackend.a ../libserver/libeibstack.a ../../common/libcommon.a
