cmake_minimum_required(VERSION 3.1)

project(jpeg9d)

set(SRCS
./cdjpeg.c
./cjpeg.c
./ckconfig.c
./djpeg.c
./example.c
./jaricom.c
./jcapimin.c
./jcapistd.c
./jcarith.c
./jccoefct.c
./jccolor.c
./jcdctmgr.c
./jchuff.c
./jcinit.c
./jcmainct.c
./jcmarker.c
./jcmaster.c
./jcomapi.c
./jcparam.c
./jcprepct.c
./jcsample.c
./jctrans.c
./jdapimin.c
./jdapistd.c
./jdarith.c
./jdatadst.c
./jdatasrc.c
./jdcoefct.c
./jdcolor.c
./jddctmgr.c
./jdhuff.c
./jdinput.c
./jdmainct.c
./jdmarker.c
./jdmaster.c
./jdmerge.c
./jdpostct.c
./jdsample.c
./jdtrans.c
./jerror.c
./jfdctflt.c
./jfdctfst.c
./jfdctint.c
./jidctflt.c
./jidctfst.c
./jidctint.c
./jmemansi.c
./jmemdos.c
./jmemmac.c
./jmemmgr.c
./jmemname.c
./jmemnobs.c
./jpegtran.c
./jquant1.c
./jquant2.c
./jutils.c
./rdbmp.c
./rdcolmap.c
./rdgif.c
./rdjpgcom.c
./rdppm.c
./rdrle.c
./rdswitch.c
./rdtarga.c
./transupp.c
./wrbmp.c
./wrgif.c
./wrjpgcom.c
./wrppm.c
./wrrle.c
./wrtarga.c)

list(FILTER SRCS EXCLUDE REGEX ".*/ckconfig.c$")
list(FILTER SRCS EXCLUDE REGEX ".*/cdjpeg.c$")
list(FILTER SRCS EXCLUDE REGEX ".*/cjpeg.c$")
list(FILTER SRCS EXCLUDE REGEX ".*/djpeg.c$")
list(FILTER SRCS EXCLUDE REGEX ".*/example.c$")
list(FILTER SRCS EXCLUDE REGEX ".*/jmemdos.c$")
list(FILTER SRCS EXCLUDE REGEX ".*/jmemmac.c$")
list(FILTER SRCS EXCLUDE REGEX ".*/jmemname.c$")
list(FILTER SRCS EXCLUDE REGEX ".*/jmemnobs.c$")
list(FILTER SRCS EXCLUDE REGEX ".*/jpegtran.c$")
list(FILTER SRCS EXCLUDE REGEX ".*/rdjpgcom.c$")
list(FILTER SRCS EXCLUDE REGEX ".*/wrjpgcom.c$")

add_library(jpeg9d STATIC ${SRCS})
set_property(TARGET jpeg9d PROPERTY POSITION_INDEPENDENT_CODE ON)
