SUBDIRS=struct configfile lib
AM_CPPFLAGS=-I../common -Istruct -Iconfigfile -Ilib -I../eibd/include
bin_PROGRAMS=viewimage bcugen1 bcugen2 loadimage imageedit

viewimage_SOURCES=viewimage.cpp
viewimage_LDADD=../common/libcommon.a lib/libbcugen.a

bcugen1_SOURCES=bcugen1.cpp
bcugen1_LDADD=../common/libcommon.a configfile/libread1.a lib/libcheck1.a lib/libbcugen.a $(XML_LIBS)

bcugen2_SOURCES=bcugen2.cpp
bcugen2_LDADD=../common/libcommon.a configfile/libread2.a lib/libcheck2.a lib/libbcugen.a $(XML_LIBS)

loadimage_SOURCES=loadimage.cpp
loadimage_LDADD=../common/libcommon.a ../eibd/client/libeibclient.a lib/libbcugen.a

imageedit_SOURCES=imageedit.cpp
imageedit_LDADD=../common/libcommon.a lib/libbcugen.a lib/libbcugen.a $(XML_LIBS)
