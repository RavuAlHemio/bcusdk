bin_PROGRAMS = eibd
AM_CPPFLAGS=-I../libserver -I../backend -I../../common
eibd_LDADD=../libserver/libeibstack.a ../backend/libbackend.a ../../common/libcommon.a
BACKEND_CONF= EIBNETIP.h FT12.h PEI16.h PEI16s.h TPUART.h TPUARTs.h
eibd_SOURCES=eibd.cpp layer2conf.h layer2create.h $(BACKEND_CONF)
