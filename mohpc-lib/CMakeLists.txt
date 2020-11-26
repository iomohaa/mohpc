cmake_minimum_required(VERSION 3.13)

project(mohpc-lib VERSION 1.0)

#file(GLOB_RECURSE SRC_ASSETS "Formats/*.cpp")
#file(GLOB_RECURSE SRC_COLLISION "Collision/*.cpp")
#file(GLOB_RECURSE SRC_COMMON "Common/*.cpp")
#file(GLOB_RECURSE SRC_NETWORK "Network/*.cpp")
#file(GLOB_RECURSE SRC_SCRIPT "Script/*.cpp")
#file(GLOB_RECURSE SRC_MISC "Utilities/*.cpp" "Platform/*.cpp" "Misc/*.cpp" "Managers/*.cpp")
#file(GLOB SRC_MISC "*.cpp" "*.h")
file(GLOB_RECURSE SRC "*.cpp")
list(FILTER SRC EXCLUDE REGEX "Platform/(.*)")

file(GLOB SRC ${SRC} "Platform/*.cpp")
file(GLOB SRC ${SRC} "Platform/Network/*.cpp")

file(GLOB_RECURSE HEADERS "${CMAKE_SOURCE_DIR}/include/*.h")

if(UNIX)
	file(GLOB_RECURSE SRC ${SRC} "Platform/Network/unix/*.cpp")
elseif(WIN32)
	file(GLOB_RECURSE SRC ${SRC} "Platform/Network/win32/*.cpp")
endif()

add_library(MOHPC SHARED ${SRC} "${CMAKE_SOURCE_DIR}/include/MOHPC/mohpc.natvis")

# Add support for std c++ filesystem on Unix
if(UNIX)
	target_link_libraries(MOHPC PUBLIC stdc++fs)
elseif(WIN32)
	target_compile_options(MOHPC PRIVATE /W3)
endif()

# Add our third-party library
target_link_libraries(MOHPC PRIVATE MOHPC-3PT)
add_dependencies(MOHPC MOHPC-3PT)

target_compile_definitions(MOHPC PRIVATE MOHPC_DLL=1 FPM_64BIT)

target_include_directories(MOHPC PRIVATE ${PROJECT_SOURCE_DIR})
target_include_directories(MOHPC PRIVATE ${PROJECT_SOURCE_DIR}/../thirdparty)
target_include_directories(MOHPC PRIVATE ${PROJECT_SOURCE_DIR}/../thirdparty/zlib)
target_include_directories(MOHPC PRIVATE ${PROJECT_SOURCE_DIR}/../thirdparty/GameSpy/src)
target_include_directories(MOHPC PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include> $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>)

set_property(TARGET MOHPC PROPERTY CMAKE_CXX_STANDARD 17)
set_property(TARGET MOHPC PROPERTY CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(TARGET MOHPC PROPERTY CMAKE_CXX_EXTENSIONS OFF)
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
