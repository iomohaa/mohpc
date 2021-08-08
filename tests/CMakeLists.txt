cmake_minimum_required(VERSION 3.13)
project(tests)

enable_testing()

include_directories("./")
add_subdirectory(Common)
add_subdirectory(Assets)
add_subdirectory(Network)