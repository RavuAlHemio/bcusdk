AM_CPPFLAGS=-I../include -I../../common

LDADD=../client/libeibclient.a
bin_PROGRAMS=busmonitor1 busmonitor2 readindividual progmodeon progmodeoff progmodetoggle progmodestatus maskver \
	writeaddress vbusmonitor1 vbusmonitor2 mprogmodeon mprogmodeoff mprogmodetoggle mprogmodestatus mmaskver \
	mpeitype madcread mread mwrite mpropread mpropwrite mpropdesc mpropscan groupread groupswrite groupwrite \
	msetkey grouplisten groupresponse groupsresponse

examplesdir=$(pkgdatadir)/examples
dist_examples_DATA=busmonitor1.c madcread.c mprogmodeoff.c mpropdesc.c mread.c progmodestatus.c vbusmonitor2.c \
	busmonitor2.c maskver.c mprogmodeon.c mpropread.c mwrite.c progmodetoggle.c writeaddress.c \
	common.c mmaskver.c mprogmodestatus.c mpropscan.c progmodeoff.c readindividual.c \
	common.h mpeitype.c mprogmodetoggle.c mpropwrite.c progmodeon.c vbusmonitor1.c groupread.c \
	groupwrite.c groupswrite.c msetkey.c grouplisten.c groupresponse.c groupsresponse.c


busmonitor1_SOURCES=busmonitor1.c common.c
busmonitor2_SOURCES=busmonitor2.c common.c
vbusmonitor1_SOURCES=vbusmonitor1.c common.c
vbusmonitor2_SOURCES=vbusmonitor2.c common.c
readindividual_SOURCES=readindividual.c common.c
progmodeon_SOURCES=progmodeon.c common.c
progmodeoff_SOURCES=progmodeoff.c common.c
progmodetoggle_SOURCES=progmodetoggle.c common.c
progmodestatus_SOURCES=progmodestatus.c common.c
maskver_SOURCES=maskver.c common.c
writeaddress_SOURCES=writeaddress.c common.c
mprogmodeon_SOURCES=mprogmodeon.c common.c
mprogmodeoff_SOURCES=mprogmodeoff.c common.c
mprogmodetoggle_SOURCES=mprogmodetoggle.c common.c
mprogmodestatus_SOURCES=mprogmodestatus.c common.c
mmaskver_SOURCES=mmaskver.c common.c
mpeitype_SOURCES=mpeitype.c common.c
madcread_SOURCES=madcread.c common.c
mread_SOURCES=mread.c common.c
mwrite_SOURCES=mwrite.c common.c
mpropread_SOURCES=mpropread.c common.c
mpropwrite_SOURCES=mpropwrite.c common.c
mpropdesc_SOURCES=mpropdesc.c common.c
mpropscan_SOURCES=mpropscan.c common.c
groupread_SOURCES=groupread.c common.c
groupwrite_SOURCES=groupwrite.c common.c
groupswrite_SOURCES=groupswrite.c common.c
groupresponse_SOURCES=groupresponse.c common.c
groupsresponse_SOURCES=groupsresponse.c common.c
grouplisten_SOURCES=grouplisten.c common.c
msetkey_SOURCES=msetkey.c common.c
