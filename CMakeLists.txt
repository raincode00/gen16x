# CMakeList.txt : CMake project for gen16x, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
project(gen16x)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
	set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
	set(SDL2_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/SDL2/")

	set(GLEW_INCLUDE_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/external/glew/include")
	#set(GLEW_LIBRARIES  "glew32s.lib")
	set(GLEW_PATH "${CMAKE_CURRENT_SOURCE_DIR}/external/glew/")
	set(GLEW_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/glew/")
endif()

add_definitions(-DGLEW_STATIC)

find_package(SDL2 REQUIRED)
find_package(GLEW REQUIRED)


include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src")

include_directories(${SDL2_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS})

file(GLOB gen16x_HEADERS 
	"${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
file(GLOB gen16x_SRC 
	"${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"  )

add_executable(gen16x ${gen16x_HEADERS} ${gen16x_SRC} "main.cpp")


target_link_libraries(gen16x ${SDL2_LIBRARIES} ${GLEW_LIBRARY})
find_package(OpenGL REQUIRED)
target_link_libraries(gen16x ${OPENGL_LIBRARIES})

if(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast -ffast-math -msse2 -fno-stack-protector")
    
    add_definitions(-DENABLE_SSE2=1)
endif()

if(APPLE)
    include_directories("/System/Library/Frameworks")
endif()

if(MSVC)
	foreach(flag_var
			CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
			CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
	   if(${flag_var} MATCHES "/MD")
		  string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
	   endif()
	endforeach()

	# set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /NODEFAULTLIB /noentry")
	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(SDL2_BINARY "${SDL2_DIR}lib/x86/SDL2.dll")
	else()
		set(SDL2_BINARY "$${SDL2_DIR}lib/x64/SDL2.dll")
	endif()
	add_custom_command(TARGET gen16x POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SDL2_BINARY}  $<TARGET_FILE_DIR:gen16x>/SDL2.dll)
endif()

