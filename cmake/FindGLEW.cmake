
if(NOT GLEW_DIR)
  set(GLEW_DIR "" CACHE PATH "GLEW directory")
endif()

find_path(GLEW_INCLUDE_DIR glew.h
  HINTS
  ${GLEW_DIR}
    ENV GLEWDIR
  PATH_SUFFIXES glew
                # path suffixes to search inside ENV{GLEWDIR}
                include/glew include
)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(VC_LIB_PATH_SUFFIX lib/Release/x64)
else()
  set(VC_LIB_PATH_SUFFIX lib/Release/Win32)
endif()


find_library(GLEW_LIBRARY_TEMP
  NAMES glew32s
  HINTS
    ${GLEW_DIR}
    ENV GLEWDIR
  PATH_SUFFIXES lib ${VC_LIB_PATH_SUFFIX}
)

# Hide this cache variable from the user, it's an internal implementation
# detail. The documented library variable for the user is GLEW_LIBRARY
# which is derived from GLEW_LIBRARY_TEMP further below.
set_property(CACHE GLEW_LIBRARY_TEMP PROPERTY TYPE INTERNAL)



if(GLEW_LIBRARY_TEMP)


  # For MinGW library
  if(MINGW)
    set(GLEW_LIBRARY_TEMP ${MINGW32_LIBRARY} ${GLEW_LIBRARY_TEMP})
  endif()

  # Set the final string here so the GUI reflects the final state.
  set(GLEW_LIBRARY ${GLEW_LIBRARY_TEMP} CACHE STRING "Where the GLEW Library can be found")
endif()


set(GLEW_LIBRARIES ${GLEW_LIBRARY})
set(GLEW_INCLUDE_DIRS ${GLEW_INCLUDE_DIR})

FIND_PACKAGE_HANDLE_STANDARD_ARGS(GLEW
                                  REQUIRED_VARS GLEW_LIBRARIES GLEW_INCLUDE_DIRS)

mark_as_advanced(GLEW_LIBRARY GLEW_INCLUDE_DIR)
