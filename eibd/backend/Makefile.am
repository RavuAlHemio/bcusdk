if HAVE_FT12
FT12 = ft12.h ft12.cpp
else
FT12 =
endif

if HAVE_PEI16
PEI16 = bcu1.h bcu1.cpp
else
PEI16 =
endif

if HAVE_TPUART
TPUART = tpuart.h tpuart.cpp
else
TPUART =
endif

if HAVE_PEI16s
PEI16s = bcu1serial.h bcu1serial.cpp
else
PEI16s =
endif

if HAVE_TPUARTs
TPUARTs = tpuartserial.h tpuartserial.cpp
else
TPUARTs =
endif

if HAVE_EIBNETIP
EIBNETIP = eibnetrouter.h eibnetrouter.cpp eibnetip.h eibnetip.cpp
else
EIBNETIP =
endif

noinst_LIBRARIES = libbackend.a
INCLUDES=-I../libserver -I../../common

libbackend_a_SOURCES= $(FT12) $(PEI16) $(TPUART) $(PEI16s) $(TPUARTs) $(EIBNETIP)

