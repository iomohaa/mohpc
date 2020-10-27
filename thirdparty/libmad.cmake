cmake_minimum_required(VERSION 3.1)

project(libmad)

set(SOURCE_DIR "libmad")

set(SRCS
${SOURCE_DIR}/bit.c
${SOURCE_DIR}/decoder.c
${SOURCE_DIR}/fixed.c
${SOURCE_DIR}/frame.c
${SOURCE_DIR}/huffman.c
${SOURCE_DIR}/layer12.c
${SOURCE_DIR}/layer3.c
${SOURCE_DIR}/minimad.c
${SOURCE_DIR}/stream.c
${SOURCE_DIR}/synth.c
${SOURCE_DIR}/timer.c
${SOURCE_DIR}/version.c
)

include_directories(${SOURCE_DIR})

list(FILTER SRCS EXCLUDE REGEX "(.*)/minimad.c$")

add_library(mad STATIC ${SRCS})
set_property(TARGET mad PROPERTY POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(mad PUBLIC FPM_64BIT)
