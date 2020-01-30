cmake_minimum_required(VERSION 3.1)

project(libmad)

set(SRCS
./bit.c
./decoder.c
./fixed.c
./frame.c
./huffman.c
./layer12.c
./layer3.c
./minimad.c
./stream.c
./synth.c
./timer.c
./version.c
)

list(FILTER SRCS EXCLUDE REGEX ".*/minimad.c$")

add_library(mad STATIC ${SRCS})
set_property(TARGET mad PROPERTY POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(mad PUBLIC FPM_64BIT)
