cmake_minimum_required(VERSION 3.1)

project(mohpc-3pt)
include(ExternalProject)

ExternalProject_Add(
	proj_jpeg9d
	URL "https://www.ijg.org/files/jpegsr9d.zip"
	CMAKE_ARGS -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
	SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/jpeg-9d"
	BINARY_DIR "./jpeg-9d"
	PATCH_COMMAND ${CMAKE_COMMAND} -E copy jconfig.txt jconfig.h && ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/jpeg9d.cmake ${CMAKE_CURRENT_SOURCE_DIR}/jpeg-9d/CMakeLists.txt
	INSTALL_COMMAND ""
)

ExternalProject_Add(
	proj_libmad
	URL "ftp://ftp.mars.org/pub/mpeg/libmad-0.15.1b.tar.gz"
	CMAKE_ARGS -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
	SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libmad"
	BINARY_DIR "./libmad"
	PATCH_COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/libmad.cmake ${CMAKE_CURRENT_SOURCE_DIR}/libmad/CMakeLists.txt
	INSTALL_COMMAND ""
)

ExternalProject_Add(
	proj_zlib
	URL "https://github.com/madler/zlib/archive/v1.2.11.zip"
	CMAKE_ARGS -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
	SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/zlib"
	BINARY_DIR "./zlib"
	PATCH_COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/zlib.cmake ${CMAKE_CURRENT_SOURCE_DIR}/zlib/CMakeLists.txt
	INSTALL_COMMAND ""
)

SET(TP_DIR ${CMAKE_BINARY_DIR}/thirdparty)

add_library(MOHPC-3PT INTERFACE)
add_dependencies(MOHPC-3PT proj_jpeg9d proj_libmad proj_zlib)

target_link_directories(MOHPC-3PT INTERFACE ${TP_DIR}/jpeg-9d ${TP_DIR}/libmad ${TP_DIR}/zlib)
target_link_libraries(MOHPC-3PT INTERFACE jpeg9d mad zlib)
