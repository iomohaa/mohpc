cmake_minimum_required(VERSION 3.1)

project(mohpc-lib VERSION 1.0)

file(GLOB_RECURSE SRC "*.cpp" "*.h")

include_directories(./)
include_directories(../thirdparty)
include_directories(../thirdparty/zlib)
include_directories(../thirdparty/GameSpy/src)

#set(EASTL_ROOT_DIR ../thirdparty/EASTL)
#include_directories (${EASTL_ROOT_DIR}/include)
#include_directories (${EASTL_ROOT_DIR}/test/packages/EAAssert/include)
#include_directories (${EASTL_ROOT_DIR}/test/packages/EABase/include/Common)
#include_directories (${EASTL_ROOT_DIR}/test/packages/EAMain/include)
#include_directories (${EASTL_ROOT_DIR}/test/packages/EAStdC/include)
#include_directories (${EASTL_ROOT_DIR}/test/packages/EATest/include)
#include_directories (${EASTL_ROOT_DIR}/test/packages/EAThread/include)
#set(EASTL_LIBRARY debug ${EASTL_ROOT_DIR}/build/Debug/EASTL.lib optimized ${EASTL_ROOT_DIR}/build/Release/EASTL.lib)
#add_custom_target(NatVis SOURCES ${EASTL_ROOT_DIR}/doc/EASTL.natvis)

add_library(MOHPC SHARED ${SRC} "Network/ClientGame.cpp")

# Add support for std c++ filesystem on Unix
if(UNIX)
	target_link_libraries(MOHPC PRIVATE stdc++fs)
endif(UNIX)

# Add our third-party library
target_link_libraries(MOHPC PRIVATE MOHPC-3PT)
add_dependencies(MOHPC MOHPC-3PT)

target_compile_definitions(MOHPC PRIVATE MOHPC_DLL=1 FPM_64BIT)
target_compile_options(MOHPC PRIVATE /W3)
set_property(TARGET MOHPC PROPERTY CXX_STANDARD 17)

add_custom_target(versioning ${CMAKE_COMMAND} -D VERSION_FILE=${CMAKE_SOURCE_DIR}/include/MOHPC/Version.h -P ${CMAKE_SOURCE_DIR}/mohpc-lib/version.cmake)
add_dependencies(MOHPC versioning)

#include(GenerateExportHeader)
#generate_export_header(MOHPC)
#add_compiler_export_flags(MOHPC_EXPORT)
#set(CMAKE_CXX_VISIBILITY_PRESET hidden)
#set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

install(TARGETS MOHPC DESTINATION .)
