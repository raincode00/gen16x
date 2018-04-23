# CMakeList.txt : CMake project for gen16x, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
project(gen16x)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
	set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
	set(SDL2_DIR "${CMAKE_SOURCE_DIR}/SDL2/")

	set(GLEW_INCLUDE_DIR  "${CMAKE_SOURCE_DIR}/glew/include")
	#set(GLEW_LIBRARIES  "glew32s.lib")
	set(GLEW_PATH "${CMAKE_SOURCE_DIR}/glew/")
	set(GLEW_DIR "${CMAKE_SOURCE_DIR}/glew/")
endif()

add_definitions(-DGLEW_STATIC)

find_package(SDL2 REQUIRED)
find_package(GLEW REQUIRED)


include_directories(${SDL2_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS})


add_executable(gen16x "gen16x.cpp" "gen16x.h" "shaders.h" "main.cpp")
# add_library(gen16x SHARED "gen16x.cpp" "gen16x.h")

target_link_libraries(gen16x ${SDL2_LIBRARIES} ${GLEW_LIBRARY})

# set( GLEW_SEARCH_PATHS
#     /usr/include
#     /usr/lib
#     /usr/local/include
#     /usr/local/lib
#     ${GLEW_PATH}
#     ${GLEW_PATH}/include
#     ${GLEW_PATH}/lib
#     ${GLEW_PATH}/lib/Release/Win32
#     ${GLEW_PATH}/lib/Debug/Win32
#     ${GLEW_PATH}/support/msvc100/Release
#     ${GLEW_PATH}/support/msvc100/Debug
#     ${GLEW_PATH}/support/msvc90/Release
#     ${GLEW_PATH}/support/msvc90/Debug
#     ${GLEW_PATH}/support/msvc80/Release
#     ${GLEW_PATH}/support/msvc80/Debug
#     )
# find_path(GLEW_INCLUDE_DIR GL/glew.h PATHS ${GLEW_SEARCH_PATHS})
# find_library(GLEW NAMES libGLEW.a glew32s.lib GLEW PATHS ${GLEW_SEARCH_PATHS})
# include_directories(${GLEW_INCLUDE_DIR})
# 
# string(REGEX REPLACE "Win32" "$(Platform)" ${GLEW} "${${GLEW}}")
# target_link_libraries(gen16x ${GLEW})

if(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffast-math -msse2 -fno-stack-protector")
    find_package(OpenGL REQUIRED)
    target_link_libraries(gen16x ${OPENGL_LIBRARIES})
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

	# set_target_properties(gen16x PROPERTIES COMPILE_FLAGS /GS-)
	# set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /NODEFAULTLIB /noentry")
	target_link_libraries(gen16x opengl32)
	add_custom_command(TARGET gen16x POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/SDL2.dll  $<TARGET_FILE_DIR:gen16x>/SDL2.dll)
endif()

