AM_CPPFLAGS=-I../../common -I../struct -I../configfile $(XML_CPPFLAGS)

noinst_LIBRARIES=libbcugen.a libcheck1.a libcheck2.a
libbcugen_a_SOURCES=common.h common.cpp regexp.h regexp.cpp symboltab.h symboltab.cpp \
	map.cpp map.h write.cpp write.h classes.h classes.cpp scanner.h \
	xmlreadconfig.cpp xmlreadconfig.h xmlwriteappinfo.cpp xmlwriteappinfo.h \
	addrtab.h addrtab.cpp expr.h expr.cpp

libcheck1_a_SOURCES=check.cpp check.h gencode.cpp gencode.h
libcheck1_a_CPPFLAGS=-D PHASE1 $(AM_CPPFLAGS)
libcheck2_a_SOURCES=check.cpp check.h gencode.cpp gencode.h
