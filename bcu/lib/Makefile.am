include ../VARIANT

AS=@TAS@
LD=@TLD@
AR=@TAR@
RANLIB=@TRANLIB@
CC=@TGCC@
CCAS=@TGCC@

tlibdir=$(pkglibdir)/lib

tlib_LIBRARIES=libbcu_0012.a libbcu_0020.a

COMMON=regs.s tmp.s fp.s tmpspace.s
BCU1=initbcu1.s bcu1func.s
BCU2=initbcu2.s bcu2func.s

libbcu_0012_a_SOURCES=$(BCU1) $(COMMON)
libbcu_0020_a_SOURCES=$(BCU2) $(COMMON)

tlib_DATA=$(addsuffix .a,$(addprefix lib,$(BCU_LIST)))

libbcu_0012%.a : libbcu_0012.a
	ln -s $< $@
libbcu_0020%.a : libbcu_0020.a
	ln -s $< $@
