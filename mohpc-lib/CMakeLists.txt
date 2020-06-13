cmake_minimum_required(VERSION 3.1)

project(mohpc-lib VERSION 1.0)

file(GLOB_RECURSE SRC "*.cpp" "*.h")

include_directories(./)
include_directories(../thirdparty)
include_directories(../thirdparty/zlib)
include_directories(../thirdparty/GameSpy/src)

add_library(MOHPC SHARED ${SRC})

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

install(TARGETS MOHPC DESTINATION .)
