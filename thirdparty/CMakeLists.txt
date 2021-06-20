cmake_minimum_required(VERSION 3.1)

project(mohpc-3pt)
include(ExternalProject)

#ExternalProject_Add(
#	proj_jpeg9d
#	URL "https://www.ijg.org/files/jpegsr9d.zip"
#	URL_HASH SHA3_512=3ae8f8bcaa9baee408322fa7ceba269f0c56afc83774f0a9bb71ff9768f547a333b7292eb9939c2a446f7a547de208b4f712746a45e9eaf7f3aec1d55cca59a7
#	CMAKE_ARGS -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
#	SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/jpeg-9d"
#	BINARY_DIR "./jpeg-9d"
#	PATCH_COMMAND ${CMAKE_COMMAND} -E copy jconfig.txt jconfig.h && ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/jpeg9d.cmake ${CMAKE_CURRENT_SOURCE_DIR}/jpeg-9d/CMakeLists.txt
#	INSTALL_COMMAND ""
#	CONFIGURE_COMMAND  ""
#	BUILD_COMMAND ""
#)
#
#ExternalProject_Add(
#	proj_libmad
#	URL "ftp://ftp.mars.org/pub/mpeg/libmad-0.15.1b.tar.gz"
#	URL_HASH SHA3_512=504dc3af19ebc7735a038e7a8294be8de008292eb578c4e5b16193d86b018b1d2caca8b30fab5f6640434e3e2458afc144a4a27ba348f91797f55d64cd205075
#	CMAKE_ARGS -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
#	SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libmad"
#	BINARY_DIR "./libmad"
#	PATCH_COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/libmad.cmake ${CMAKE_CURRENT_SOURCE_DIR}/libmad/CMakeLists.txt
#	INSTALL_COMMAND ""
#	CONFIGURE_COMMAND  ""
#	BUILD_COMMAND ""
#)
#
#ExternalProject_Add(
#	proj_zlib
#	URL "https://github.com/madler/zlib/archive/v1.2.11.zip"
#	URL_HASH SHA3_512=7df35a6e3c2eccfc7ac218c3040827375df7a18ca04503538086290d3954cdbacf5a5de69efadb626f14f7d9f2255432721f32a705fdf12ba9731f911cb7a6a9
#	CMAKE_ARGS -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
#	SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/zlib"
#	BINARY_DIR "./zlib"
#	PATCH_COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/zlib.cmake ${CMAKE_CURRENT_SOURCE_DIR}/zlib/CMakeLists.txt
#	INSTALL_COMMAND ""
#	CONFIGURE_COMMAND  ""
#	BUILD_COMMAND ""
#)
#
#ExternalProject_Add(
#	proj_gcd
#	URL "https://github.com/nitrocaster/GameSpy/archive/master.zip"
#	CMAKE_ARGS -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
#	SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/GameSpy"
#	BINARY_DIR "./GameSpy"
#	PATCH_COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/gcd.cmake ${CMAKE_CURRENT_SOURCE_DIR}/GameSpy/CMakeLists.txt
#	INSTALL_COMMAND ""
#	CONFIGURE_COMMAND  ""
#	BUILD_COMMAND ""
#)

include("gcd.cmake")
include("jpeg9d.cmake")
include("libmad.cmake")
include("zlib.cmake")

SET(TP_DIR ${CMAKE_BINARY_DIR}/thirdparty)

add_library(MOHPC-3PT INTERFACE)
add_dependencies(MOHPC-3PT proj_jpeg9d proj_libmad proj_zlib proj_gcd)

target_link_directories(MOHPC-3PT INTERFACE ${TP_DIR}/jpeg-9d ${TP_DIR}/libmad ${TP_DIR}/zlib ${TP_DIR}/GameSpy)
target_link_libraries(MOHPC-3PT INTERFACE jpeg9d mad zlib gcd)
