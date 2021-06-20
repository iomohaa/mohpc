cmake_minimum_required(VERSION 3.13)

project(mohpc-lib VERSION 1.0)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_subdirectory(Assets)
add_subdirectory(Files)
add_subdirectory(Utility)
add_subdirectory(Network)

find_package(morfuse)

file(GLOB_RECURSE SRC "Common/*.cpp")

add_library(MOHPC SHARED ${SRC} "${CMAKE_SOURCE_DIR}/include/MOHPC/mohpc.natvis")
target_sources(MOHPC PRIVATE $<TARGET_OBJECTS:MOHPC-Assets>)
target_sources(MOHPC PRIVATE $<TARGET_OBJECTS:MOHPC-Files>)
target_sources(MOHPC PRIVATE $<TARGET_OBJECTS:MOHPC-Utility>)
target_sources(MOHPC PRIVATE $<TARGET_OBJECTS:MOHPC-Net>)

# Add support for std c++ filesystem on Unix
if(UNIX)
	target_link_libraries(MOHPC PUBLIC stdc++fs)
elseif(WIN32)
	target_compile_options(MOHPC PRIVATE "/W3")
	# disable warnings about defined symbol imported by other objects
	target_link_options(MOHPC PRIVATE "/ignore:4217" "/ignore:4286")
endif()

add_library(MOHPC-ALL INTERFACE)
target_link_libraries(MOHPC-ALL INTERFACE MOHPC)

# Add our third-party library
target_link_libraries(MOHPC PRIVATE MOHPC-3PT)
target_link_libraries(MOHPC PUBLIC mfuse::morfuse)
add_dependencies(MOHPC MOHPC-3PT)

target_compile_definitions(MOHPC PRIVATE MOHPC_DLL=1)

target_include_directories(MOHPC PUBLIC ${morfuse_INCLUDE_DIR})
target_include_directories(MOHPC PRIVATE ${PROJECT_SOURCE_DIR})
target_include_directories(MOHPC PRIVATE ${PROJECT_SOURCE_DIR}/../thirdparty)
target_include_directories(MOHPC PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include> $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>)
target_compile_features(MOHPC PUBLIC cxx_std_17)

add_custom_target(versioning ${CMAKE_COMMAND} -D VERSION_FILE=${CMAKE_SOURCE_DIR}/include/MOHPC/Version.h -P ${CMAKE_SOURCE_DIR}/mohpc-lib/version.cmake)
add_dependencies(MOHPC versioning)

install(
	TARGETS MOHPC
	DESTINATION ${CMAKE_INSTALL_PREFIX}
	EXPORT MOHPC-Targets
)

install(
	DIRECTORY "${CMAKE_SOURCE_DIR}/include/MOHPC"
	DESTINATION include
)

install(
	EXPORT MOHPC-Targets
	DESTINATION .
	NAMESPACE MOHPC::
)

set(INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/include)
set(LIB_INSTALL_DIR ${CMAKE_INSTALL_PREFIX})

include(CMakePackageConfigHelpers)
configure_package_config_file( 
  "Config.cmake.in" 
  "MOHPC-Config.cmake"
  INSTALL_DESTINATION ${CMAKE_INSTALL_PREFIX}
  PATH_VARS
    INCLUDE_INSTALL_DIR
	LIB_INSTALL_DIR
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/MOHPC-Config.cmake DESTINATION ${CMAKE_INSTALL_PREFIX})

if(MSVC)
	install(
		FILES $<TARGET_PDB_FILE:MOHPC>
		DESTINATION .
		OPTIONAL
	)
endif()
