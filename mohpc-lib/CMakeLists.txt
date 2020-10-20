cmake_minimum_required(VERSION 3.1)

project(mohpc-lib VERSION 1.0)

file(GLOB_RECURSE SRC "*.cpp" "*.h")
file(GLOB_RECURSE HEADERS "${CMAKE_SOURCE_DIR}/include/*.h")

add_library(MOHPC SHARED ${SRC} "${CMAKE_SOURCE_DIR}/include/MOHPC/mohpc.natvis")

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/../thirdparty)
include_directories(${PROJECT_SOURCE_DIR}/../thirdparty/zlib)
include_directories(${PROJECT_SOURCE_DIR}/../thirdparty/GameSpy/src)

# Add support for std c++ filesystem on Unix
if(UNIX)
	target_link_libraries(MOHPC PRIVATE stdc++fs)
endif(UNIX)

# Add our third-party library
target_link_libraries(MOHPC PRIVATE MOHPC-3PT)
add_dependencies(MOHPC MOHPC-3PT)

target_compile_definitions(MOHPC PRIVATE MOHPC_DLL=1 FPM_64BIT)
target_compile_options(MOHPC PRIVATE /W3)
target_include_directories(MOHPC PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include> $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>)
set_property(TARGET MOHPC PROPERTY CXX_STANDARD 17)

add_custom_target(versioning ${CMAKE_COMMAND} -D VERSION_FILE=${CMAKE_SOURCE_DIR}/include/MOHPC/Version.h -P ${CMAKE_SOURCE_DIR}/mohpc-lib/version.cmake)
add_dependencies(MOHPC versioning)

install(
	TARGETS MOHPC
	EXPORT MOHPC-Config
	DESTINATION "${CMAKE_INSTALL_PREFIX}"
	INCLUDES DESTINATION "${CMAKE_INSTALL_PREFIX}/include"
)

install(
	DIRECTORY "${CMAKE_SOURCE_DIR}/include/MOHPC"
	DESTINATION include
)

install(
	FILES $<TARGET_PDB_FILE:MOHPC>
	DESTINATION "${CMAKE_INSTALL_PREFIX}"
)

export(
	TARGETS MOHPC
	NAMESPACE MOHPC::
	FILE "${CMAKE_INSTALL_PREFIX}/mohpc-config.cmake"
)

export(
	PACKAGE MOHPC
)

install(
	EXPORT MOHPC-Config
	DESTINATION "${CMAKE_INSTALL_PREFIX}"
	NAMESPACE MOHPC::
)


