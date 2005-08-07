bin_PROGRAMS = eibd
AM_CPPFLAGS=-I../libserver -I../backend -I../../common -I../usb
eibd_LDADD=../backend/libbackend.a ../libserver/libeibstack.a ../../common/libcommon.a ../usb/libusb.a
BACKEND_CONF= b-EIBNETIP.h b-FT12.h b-PEI16.h b-PEI16s.h b-TPUART.h b-TPUARTs.h b-EIBNETIPTUNNEL.h b-USB.h
eibd_SOURCES=eibd.cpp layer2conf.h layer2create.h $(BACKEND_CONF)
