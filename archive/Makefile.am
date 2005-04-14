bin_PROGRAMS=embedprogid extractprogid
AM_CPPFLAGS=-I../common -I../bcugen/lib -I../bcugen/struct $(XML_CPPFLAGS)

embedprogid_SOURCES=embedprogid.cpp hex.cpp hex.h
embedprogid_LDADD=../bcugen/lib/libbcugen.a $(XML_LIBS)

extractprogid_SOURCES=extractprogid.cpp hex.cpp hex.h
extractprogid_LDADD=../bcugen/lib/libbcugen.a $(XML_LIBS)


