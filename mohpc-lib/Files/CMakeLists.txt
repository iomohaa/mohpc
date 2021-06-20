cmake_minimum_required(VERSION 3.13)

project(MOHPC-FileLibrary VERSION 1.0)

find_package(morfuse)

file(GLOB_RECURSE SRC "*.cpp")

add_library(MOHPC-Files OBJECT ${SRC})

target_link_directories(MOHPC-Files PRIVATE ${morfuse_LIBS})
target_compile_definitions(MOHPC-Files PRIVATE MOHPC_FILES_DLL=1)

target_include_directories(MOHPC-Files PUBLIC ${morfuse_INCLUDE_DIR})
target_include_directories(MOHPC-Files PRIVATE ${PROJECT_SOURCE_DIR}/..)
target_include_directories(MOHPC-Files PRIVATE ${PROJECT_SOURCE_DIR}/../../thirdparty)
target_include_directories(MOHPC-Files PRIVATE ${PROJECT_SOURCE_DIR}/../../thirdparty/zlib)
target_compile_features(MOHPC-Files PUBLIC cxx_std_17)
