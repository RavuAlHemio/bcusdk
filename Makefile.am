AUTOMAKE_OPTIONS=1.9

SUBDIRS=. common eibd xml bcu bcugen archive

BUILT_SOURCES=path.h
CLEANFILES=path.h

path.h : Makefile
	echo "#define PKGDATADIR \"${pkgdatadir}\"" >$@
	echo "#define PKGLIBDIR \"${pkglibdir}\"" >>$@
