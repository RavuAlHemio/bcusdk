include ../VARIANT

BCU1=bcu_0012.link
BCU2=bcu_0020.link bcu_0020histack.link

lddir=$(pkgdatadir)/ldscripts
dist_ld_DATA=genload.link $(BCU1) $(BCU2)

install-data-hook:
	$(mkinstalldirs) $(DESTDIR)$(lddir)
	for a in $(addsuffix .link,$(BCU_LIST)) ; do ../addmissing.sh $(DESTDIR)$(lddir) $$a; done
