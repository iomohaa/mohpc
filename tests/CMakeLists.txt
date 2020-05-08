cmake_minimum_required(VERSION 3.1)

project(test)

file(GLOB_RECURSE SRC "*")

add_executable(MOHPC-Tests ${SRC})
target_link_libraries(MOHPC-Tests PUBLIC MOHPC)
set_property(TARGET MOHPC-Tests PROPERTY CXX_STANDARD 17)

install(TARGETS MOHPC-Tests DESTINATION .)
