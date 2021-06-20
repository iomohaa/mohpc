cmake_minimum_required(VERSION 3.13)
project(tests)

enable_testing()

file(GLOB SRCS "*.cpp")
file(GLOB_RECURSE COMMON_SRCS "Common/*.cpp" "Common/*.h")

find_package(Threads REQUIRED)

foreach(source ${SRCS})
	get_filename_component(fileName ${source} NAME_WE)

	# Add executable with only the source file
	add_executable(${fileName} ${source} ${COMMON_SRCS})

	target_link_libraries(${fileName} MOHPC-ALL Threads::Threads)

	add_test(NAME ${fileName} COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/${fileName})

	install(
		TARGETS ${fileName}
		DESTINATION ${CMAKE_INSTALL_PREFIX}
		EXPORT test-Targets
	)
endforeach(source)

install(
	EXPORT MOHPC-Targets
	DESTINATION .
	NAMESPACE MOHPC::
)
