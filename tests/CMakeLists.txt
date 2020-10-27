cmake_minimum_required(VERSION 3.10)

project(test)

file(GLOB_RECURSE SRC "*")

add_executable(MOHPC-Tests ${SRC} "${CMAKE_SOURCE_DIR}/include/MOHPC/mohpc.natvis")
target_link_libraries(MOHPC-Tests PUBLIC MOHPC)

if(UNIX)
	target_link_libraries(MOHPC-Tests PRIVATE ncurses pthread)
endif()

install(TARGETS MOHPC-Tests DESTINATION .)
