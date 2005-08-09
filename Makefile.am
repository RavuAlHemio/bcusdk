AUTOMAKE_OPTIONS=1.9
EXTRA_DIST=bcusdk.spec
SUBDIRS=. common eibd xml bcu bcugen archive build debian

BUILT_SOURCES=path.h
CLEANFILES=path.h

path.h : Makefile
	echo "#define PKGDATADIR \"${pkgdatadir}\"" >$@
	echo "#define PKGLIBDIR \"${pkglibdir}\"" >>$@
