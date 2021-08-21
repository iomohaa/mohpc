cmake_minimum_required(VERSION 3.13)
project(tests_common)

file(GLOB_RECURSE COMMON_SRCS "../Common/*.cpp" "../Common/*.h")

# Add executable with only the source file
add_library(TESTS_COMMON STATIC ${COMMON_SRCS})
target_link_libraries(TESTS_COMMON MOHPC-ALL)
target_include_directories(TESTS_COMMON PRIVATE "../../thirdparty/tclap/include")

if (WIN32)
	# For timer resolution
	target_link_libraries(TESTS_COMMON Winmm)
endif()

install(
	TARGETS ${fileName}
	DESTINATION ${CMAKE_INSTALL_PREFIX}
	EXPORT testCommon-Targets
)

install(
	EXPORT MOHPC-Targets
	DESTINATION .
	NAMESPACE MOHPC::
)
