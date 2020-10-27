cmake_minimum_required(VERSION 3.1)

project(jpeg9d)

set(SOURCE_DIR "jpeg-9d")

set(SRCS
${SOURCE_DIR}/cdjpeg.c
${SOURCE_DIR}/cjpeg.c
${SOURCE_DIR}/ckconfig.c
${SOURCE_DIR}/djpeg.c
${SOURCE_DIR}/example.c
${SOURCE_DIR}/jaricom.c
${SOURCE_DIR}/jcapimin.c
${SOURCE_DIR}/jcapistd.c
${SOURCE_DIR}/jcarith.c
${SOURCE_DIR}/jccoefct.c
${SOURCE_DIR}/jccolor.c
${SOURCE_DIR}/jcdctmgr.c
${SOURCE_DIR}/jchuff.c
${SOURCE_DIR}/jcinit.c
${SOURCE_DIR}/jcmainct.c
${SOURCE_DIR}/jcmarker.c
${SOURCE_DIR}/jcmaster.c
${SOURCE_DIR}/jcomapi.c
${SOURCE_DIR}/jcparam.c
${SOURCE_DIR}/jcprepct.c
${SOURCE_DIR}/jcsample.c
${SOURCE_DIR}/jctrans.c
${SOURCE_DIR}/jdapimin.c
${SOURCE_DIR}/jdapistd.c
${SOURCE_DIR}/jdarith.c
${SOURCE_DIR}/jdatadst.c
${SOURCE_DIR}/jdatasrc.c
${SOURCE_DIR}/jdcoefct.c
${SOURCE_DIR}/jdcolor.c
${SOURCE_DIR}/jddctmgr.c
${SOURCE_DIR}/jdhuff.c
${SOURCE_DIR}/jdinput.c
${SOURCE_DIR}/jdmainct.c
${SOURCE_DIR}/jdmarker.c
${SOURCE_DIR}/jdmaster.c
${SOURCE_DIR}/jdmerge.c
${SOURCE_DIR}/jdpostct.c
${SOURCE_DIR}/jdsample.c
${SOURCE_DIR}/jdtrans.c
${SOURCE_DIR}/jerror.c
${SOURCE_DIR}/jfdctflt.c
${SOURCE_DIR}/jfdctfst.c
${SOURCE_DIR}/jfdctint.c
${SOURCE_DIR}/jidctflt.c
${SOURCE_DIR}/jidctfst.c
${SOURCE_DIR}/jidctint.c
${SOURCE_DIR}/jmemansi.c
${SOURCE_DIR}/jmemdos.c
${SOURCE_DIR}/jmemmac.c
${SOURCE_DIR}/jmemmgr.c
${SOURCE_DIR}/jmemname.c
${SOURCE_DIR}/jmemnobs.c
${SOURCE_DIR}/jpegtran.c
${SOURCE_DIR}/jquant1.c
${SOURCE_DIR}/jquant2.c
${SOURCE_DIR}/jutils.c
${SOURCE_DIR}/rdbmp.c
${SOURCE_DIR}/rdcolmap.c
${SOURCE_DIR}/rdgif.c
${SOURCE_DIR}/rdjpgcom.c
${SOURCE_DIR}/rdppm.c
${SOURCE_DIR}/rdrle.c
${SOURCE_DIR}/rdswitch.c
${SOURCE_DIR}/rdtarga.c
${SOURCE_DIR}/transupp.c
${SOURCE_DIR}/wrbmp.c
${SOURCE_DIR}/wrgif.c
${SOURCE_DIR}/wrjpgcom.c
${SOURCE_DIR}/wrppm.c
${SOURCE_DIR}/wrrle.c
${SOURCE_DIR}/wrtarga.c)

include_directories(${SOURCE_DIR})

list(FILTER SRCS EXCLUDE REGEX "(.*)/ckconfig.c$")
list(FILTER SRCS EXCLUDE REGEX "(.*)/cdjpeg.c$")
list(FILTER SRCS EXCLUDE REGEX "(.*)/cjpeg.c$")
list(FILTER SRCS EXCLUDE REGEX "(.*)/djpeg.c$")
list(FILTER SRCS EXCLUDE REGEX "(.*)/example.c$")
list(FILTER SRCS EXCLUDE REGEX "(.*)/jmemdos.c$")
list(FILTER SRCS EXCLUDE REGEX "(.*)/jmemmac.c$")
list(FILTER SRCS EXCLUDE REGEX "(.*)/jmemname.c$")
list(FILTER SRCS EXCLUDE REGEX "(.*)/jmemnobs.c$")
list(FILTER SRCS EXCLUDE REGEX "(.*)/jpegtran.c$")
list(FILTER SRCS EXCLUDE REGEX "(.*)/rdjpgcom.c$")
list(FILTER SRCS EXCLUDE REGEX "(.*)/wrjpgcom.c$")

add_library(jpeg9d STATIC ${SRCS})
target_compile_definitions(jpeg9d PRIVATE _CRT_SECURE_NO_WARNINGS)
set_property(TARGET jpeg9d PROPERTY POSITION_INDEPENDENT_CODE ON)
