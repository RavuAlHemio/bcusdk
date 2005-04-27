include ../VARIANT

BCU1=bcu_0012.link
BCU2=bcu_0020.link

lddir=$(pkgdatadir)/ldscripts
dist_ld_DATA=genload.link $(BCU1) $(BCU2)
ld_DATA=$(addsuffix .link,$(BCU_LIST))

bcu_0012%.link : bcu_0012.link
	-ln -s $< $@
bcu_0020%.link : bcu_0020.link
	-ln -s $< $@
