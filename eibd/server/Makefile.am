bin_PROGRAMS = eibd
AM_CPPFLAGS=-I../libserver -I../backend -I../../common
eibd_LDADD=../libserver/libeibstack.a ../backend/libbackend.a ../../common/libcommon.a
BACKEND_CONF= b-EIBNETIP.h b-FT12.h b-PEI16.h b-PEI16s.h b-TPUART.h b-TPUARTs.h b-EIBNETIPTUNNEL.h
eibd_SOURCES=eibd.cpp layer2conf.h layer2create.h $(BACKEND_CONF)
