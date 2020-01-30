cmake_minimum_required(VERSION 3.1)

project(zlib)

set(SRCS
./contrib/minizip/ioapi.c
./contrib/minizip/iowin32.c
./contrib/minizip/miniunz.c
./contrib/minizip/minizip.c
./contrib/minizip/mztools.c
./contrib/minizip/zip.c
./adler32.c
./compress.c
./crc32.c
./deflate.c
./gzclose.c
./gzlib.c
./gzread.c
./gzwrite.c
./infback.c
./inffast.c
./inflate.c
./inftrees.c
./trees.c
./uncompr.c
./zutil.c
)

set(PATCH_SRCS ../zlib_unzip_patch.c)

list(FILTER SRCS EXCLUDE REGEX ".*/miniunz.c$")
list(FILTER SRCS EXCLUDE REGEX ".*/minizip.c$")
list(FILTER SRCS EXCLUDE REGEX ".*/iowin32.c$")
list(FILTER SRCS EXCLUDE REGEX ".*/zip.c$")

add_library(zlib STATIC ${SRCS} ${PATCH_SRCS})
set_property(TARGET zlib PROPERTY POSITION_INDEPENDENT_CODE ON)
target_include_directories(zlib PUBLIC ./)
