cmake_minimum_required(VERSION 3.13)
project(tests_network)

file(GLOB SRCS "*.cpp")

find_package(Threads REQUIRED)

add_subdirectory(${glfw3_DIR} bin)

foreach(source ${SRCS})
	get_filename_component(fileName ${source} NAME_WE)

	# Add executable with only the source file
	add_executable(${fileName} ${source} ${COMMON_SRCS})
	
	#target_include_directories(${fileName} PUBLIC ${glfw3_INCLUDE})
	target_link_libraries(${fileName} MOHPC-ALL TESTS_COMMON glfw ${glfw_LIBRARIES} Threads::Threads)

	add_test(NAME ${fileName} COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/${fileName})

	install(
		TARGETS ${fileName}
		DESTINATION ${CMAKE_INSTALL_PREFIX}
		EXPORT testNetwork-Targets
	)
endforeach(source)

install(
	EXPORT MOHPC-Targets
	DESTINATION .
	NAMESPACE MOHPC::
)
