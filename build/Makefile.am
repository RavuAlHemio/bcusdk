BINDIR=$(bindir)
LIBDIR=$(pkglibdir)/lib
LDDIR=$(pkglibdir)/ldscripts
INCDIR=$(pkgdatadir)/include
XMLDIR=$(pkgdatadir)/xml

do_subst = sed -e 's,[@]VERSION[@],$(VERSION),g' \
                 -e 's,[@]TAS[@],$(TAS),g' \
                 -e 's,[@]TLD[@],$(TLD),g' \
                 -e 's,[@]TGCC[@],$(TGCC),g' \
                 -e 's,[@]TRANLIB[@],$(TRANLIB),g' \
                 -e 's,[@]TAR[@],$(TAR),g' \
                 -e 's,[@]LDDIR[@],$(LDDIR),g' \
                 -e 's,[@]LIBDIR[@],$(LIBDIR),g' \
                 -e 's,[@]INCDIR[@],$(INCDIR),g' \
                 -e 's,[@]BINDIR[@],$(BINDIR),g' \
                 -e 's,[@]XMLDIR[@],$(XMLDIR),g'


bin_SCRIPTS=build.ai build.img gencitemplate
CLEANFILES=build.ai build.img gencitemplate

% : $(srcdir)/%.in Makefile
	$(do_subst) < $<  > $@
	chmod a+x $@

