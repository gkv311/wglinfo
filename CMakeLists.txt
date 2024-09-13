cmake_minimum_required (VERSION 3.2)

project (wglinfo)

set (APP_VERSION_MAJOR 1)
set (APP_VERSION_MINOR 0)

# compiler flags
set (CMAKE_CXX_STANDARD 11)
if (MSVC)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /fp:precise /EHa /MP")
  string (REGEX REPLACE "/EHsc" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  add_definitions (-D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_DEPRECATE -DUNICODE)
else()
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions -fPIC")
endif()

set (BUILD_TREAT_WARNINGS_AS_ERRORS OFF CACHE BOOL "Treat compilation warnings as errors")

# increase compiler warnings level (-W3 for MSVC, -Wextra for GCC)
if (MSVC)
  if (CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string (REGEX REPLACE "/W[0-4]" "/W3" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3")
  endif()
  if (BUILD_TREAT_WARNINGS_AS_ERRORS)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX")    
  endif()
elseif (CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
  if (BUILD_TREAT_WARNINGS_AS_ERRORS)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")    
  endif()
endif()

# Find OpenGL
find_package (OpenGL REQUIRED)

# main project target
add_executable (${PROJECT_NAME}
  wglinfo.cpp
)
target_link_libraries (${PROJECT_NAME} PRIVATE ${OPENGL_LIBRARIES})
