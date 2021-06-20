cmake_minimum_required(VERSION 3.13)

project(MOHPC-CollisionLib VERSION 1.0)

find_package(morfuse)

file(GLOB_RECURSE SRC "*.cpp")

add_library(MOHPC-Collision OBJECT ${SRC})

target_link_directories(MOHPC-Collision PRIVATE ${morfuse_LIBS})
target_compile_definitions(MOHPC-Collision PRIVATE MOHPC_COLLISION_DLL=1)

target_include_directories(MOHPC-Collision PUBLIC ${morfuse_INCLUDE_DIR})
target_include_directories(MOHPC-Collision PRIVATE ${PROJECT_SOURCE_DIR}/..)
target_include_directories(MOHPC-Collision PRIVATE ${PROJECT_SOURCE_DIR}/../../thirdparty)
target_compile_features(MOHPC-Collision PUBLIC cxx_std_17)
