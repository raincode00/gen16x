# CMakeList.txt : CMake project for gen16x, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.


set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

if(WIN32)
	if(MSVC)
		set(SDL2_DIR "${CMAKE_SOURCE_DIR}/SDL2/")
		set(GLEW_INCLUDE_DIR  "${CMAKE_SOURCE_DIR}/glew/include")
		set(GLEW_LIBRARY  "${CMAKE_SOURCE_DIR}/glew/lib/Release/Win32/glew32s.lib")
	endif()
endif()
add_definitions(-DGLEW_STATIC)

find_package(SDL2 REQUIRED)
find_package(GLEW REQUIRED)

include_directories(${SDL2_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS})


add_executable (gen16x "gen16x.cpp" "gen16x.h")

target_link_libraries(gen16x ${SDL2_LIBRARIES})
target_link_libraries(gen16x opengl32 ${GLEW_LIBRARIES})
    
if(WIN32)

	if(MSVC)
		add_definitions(/arch:SSE2)
		add_custom_command(TARGET gen16x POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/SDL2.dll ${CMAKE_BINARY_DIR}/SDL2.dll)

	endif()
endif()