include ../VARIANT

AS=@TAS@
LD=@TLD@
AR=@TAR@
RANLIB=@TRANLIB@
CC=@TGCC@
CCAS=@TGCC@
AM_CPPFLAGS=-I$(top_srcdir)/bcu/include
AM_CFLAGS=-gstabs -O3 -Os
CFLAGS=
EXTRA_DIST=eewrite.c

tlibdir=$(pkglibdir)/lib

tlib_LIBRARIES=libbcu_0012.a libbcu_0020.a

COMMON=regs.s tmp.s fp.s tmpspace.s
BCU1=initbcu1.s bcu1func.s
BCU2=initbcu2.s bcu2func.s
BCU1_EE=eewrite_QI.c eewrite_HI.c eewrite_AI.c eewrite_SI.c \
	eewrite_FI.c eewrite_CI.c eewrite_EI.c eewrite_DI.c

BUILT_SOURCES=$(BCU1_EE)
CLEANFILES=$(BCU1_EE)

libbcu_0012_a_SOURCES=$(BCU1) $(COMMON) $(BCU1_EE)
libbcu_0020_a_SOURCES=$(BCU2) $(COMMON) $(BCU1_EE)

$(BCU1_EE) : Makefile
	for b in QI HI AI SI EI CI FI DI ; do \
	echo "#define MODE $$b" >eewrite_$$b.c ; \
	echo "#include \"eewrite.c\"" >>eewrite_$$b.c ; \
	done

install-data-hook:
	$(mkinstalldirs) $(DESTDIR)$(tlibdir)
	for a in $(addprefix lib,$(addsuffix .a,$(BCU_LIST))) ; do $(top_srcdir)/bcu/addmissing.sh $(DESTDIR)$(tlibdir) $$a; done
