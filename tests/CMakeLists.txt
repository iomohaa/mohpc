cmake_minimum_required(VERSION 3.1)

project(test)

file(GLOB_RECURSE SRC "*")

add_executable(MOHPC-Tests ${SRC} "${CMAKE_SOURCE_DIR}/include/MOHPC/mohpc.natvis")
target_link_libraries(MOHPC-Tests PUBLIC MOHPC)
set_property(TARGET MOHPC-Tests PROPERTY CXX_STANDARD 17)

install(TARGETS MOHPC-Tests DESTINATION .)
