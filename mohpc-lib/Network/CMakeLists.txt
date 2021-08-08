cmake_minimum_required(VERSION 3.13)

project(MOHPC-Netlib VERSION 1.0)

file(GLOB_RECURSE SRC "*.cpp")
list(FILTER SRC EXCLUDE REGEX "Platform/(.*)")

file(GLOB SRC ${SRC} "Platform/*.cpp")

file(GLOB_RECURSE HEADERS "${CMAKE_SOURCE_DIR}/include/*.h")

if(UNIX)
	file(GLOB_RECURSE SRC ${SRC} "Platform/unix/*.cpp")
elseif(WIN32)
	file(GLOB_RECURSE SRC ${SRC} "Platform/win32/*.cpp")
endif()

add_library(MOHPC-Net OBJECT ${SRC})

target_compile_definitions(MOHPC-Net PRIVATE MOHPC_DLL=1 MOHPC_NET_DLL=1)

target_include_directories(MOHPC-Net PRIVATE ${Eigen_DIR})
target_include_directories(MOHPC-Net PRIVATE ${PROJECT_SOURCE_DIR}/../../thirdparty/GameSpy/src)
target_compile_features(MOHPC-Net PUBLIC cxx_std_17)
