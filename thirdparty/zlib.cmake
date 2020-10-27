cmake_minimum_required(VERSION 3.1)

project(zlib)

set(SOURCE_DIR "zlib")

set(SRCS
${SOURCE_DIR}/contrib/minizip/ioapi.c
${SOURCE_DIR}/contrib/minizip/iowin32.c
${SOURCE_DIR}/contrib/minizip/miniunz.c
${SOURCE_DIR}/contrib/minizip/minizip.c
${SOURCE_DIR}/contrib/minizip/mztools.c
${SOURCE_DIR}/contrib/minizip/zip.c
${SOURCE_DIR}/adler32.c
${SOURCE_DIR}/compress.c
${SOURCE_DIR}/crc32.c
${SOURCE_DIR}/deflate.c
${SOURCE_DIR}/gzclose.c
${SOURCE_DIR}/gzlib.c
${SOURCE_DIR}/gzread.c
${SOURCE_DIR}/gzwrite.c
${SOURCE_DIR}/infback.c
${SOURCE_DIR}/inffast.c
${SOURCE_DIR}/inflate.c
${SOURCE_DIR}/inftrees.c
${SOURCE_DIR}/trees.c
${SOURCE_DIR}/uncompr.c
${SOURCE_DIR}/zutil.c
)

set(PATCH_SRCS zlib_unzip_patch.c)

include_directories(${SOURCE_DIR})

list(FILTER SRCS EXCLUDE REGEX "(.*)/miniunz.c$")
list(FILTER SRCS EXCLUDE REGEX "(.*)/minizip.c$")
list(FILTER SRCS EXCLUDE REGEX "(.*)/iowin32.c$")
list(FILTER SRCS EXCLUDE REGEX "(.*)/zip.c$")

add_library(zlib STATIC ${SRCS} ${PATCH_SRCS})
set_property(TARGET zlib PROPERTY POSITION_INDEPENDENT_CODE ON)
target_include_directories(zlib PUBLIC ./)
