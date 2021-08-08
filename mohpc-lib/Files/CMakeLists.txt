cmake_minimum_required(VERSION 3.13)

project(MOHPC-FileLibrary VERSION 1.0)

file(GLOB_RECURSE SRC "*.cpp")

add_library(MOHPC-Files OBJECT ${SRC})

target_compile_definitions(MOHPC-Files PRIVATE MOHPC_FILES_DLL=1)

target_include_directories(MOHPC-Files PRIVATE ${PROJECT_SOURCE_DIR}/..)
target_include_directories(MOHPC-Files PRIVATE ${PROJECT_SOURCE_DIR}/../../thirdparty)
target_include_directories(MOHPC-Files PRIVATE ${PROJECT_SOURCE_DIR}/../../thirdparty/zlib)
target_compile_features(MOHPC-Files PUBLIC cxx_std_17)
