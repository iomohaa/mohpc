cmake_minimum_required(VERSION 3.13)

project(MOHPC-AssetsLib VERSION 1.0)

file(GLOB_RECURSE SRC "*.cpp")

add_library(MOHPC-Assets OBJECT ${SRC})

target_compile_definitions(MOHPC-Assets PRIVATE MOHPC_ASSETS_DLL=1 FPM_64BIT)

target_include_directories(MOHPC-Assets PRIVATE ${PROJECT_SOURCE_DIR}/..)
target_include_directories(MOHPC-Assets PUBLIC ${Eigen_DIR})
target_include_directories(MOHPC-Assets PRIVATE ${PROJECT_SOURCE_DIR}/../../thirdparty)
target_compile_features(MOHPC-Assets PUBLIC cxx_std_17)
