SUBDIRS=struct configfile lib
INCLUDES=-I../common -Istruct -Iconfigfile -Ilib
bin_PROGRAMS=viewimage bcugen1 bcugen2

viewimage_SOURCES=viewimage.cpp
viewimage_LDADD=../common/libcommon.a lib/libbcugen.a

bcugen1_SOURCES=bcugen1.cpp
bcugen1_LDADD=../common/libcommon.a configfile/libread1.a lib/libcheck1.a lib/libbcugen.a $(XML_LIBS)

bcugen2_SOURCES=bcugen2.cpp
bcugen2_LDADD=../common/libcommon.a configfile/libread2.a lib/libcheck2.a lib/libbcugen.a $(XML_LIBS)

