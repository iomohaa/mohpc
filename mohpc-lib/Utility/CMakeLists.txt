cmake_minimum_required(VERSION 3.13)

project(MOHPC-Utilitylib VERSION 1.0)

find_package(morfuse)

file(GLOB_RECURSE SRC "*.cpp")

add_library(MOHPC-Utility OBJECT ${SRC})

target_compile_definitions(MOHPC-Utility PRIVATE MOHPC_UTILITY_DLL=1)

target_include_directories(MOHPC-Utility PRIVATE ${Eigen_DIR})
target_include_directories(MOHPC-Utility PRIVATE ${morfuse_INCLUDE_DIR})
target_include_directories(MOHPC-Utility PRIVATE ${PROJECT_SOURCE_DIR}/..)
target_include_directories(MOHPC-Utility PRIVATE ${PROJECT_SOURCE_DIR}/../../thirdparty)
target_link_directories(MOHPC-Utility PRIVATE ${morfuse_LIBS})
target_compile_features(MOHPC-Utility PUBLIC cxx_std_17)
