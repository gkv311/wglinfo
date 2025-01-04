cmake_minimum_required (VERSION 3.2)

project (wglinfo)

set (APP_VERSION_MAJOR 1)
set (APP_VERSION_MINOR 0)

# compiler flags
set (CMAKE_CXX_STANDARD 11)
if (MSVC)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /fp:precise /EHa /MP")
  string (REGEX REPLACE "/EHsc" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  add_definitions (-D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_DEPRECATE)
  # static linking with CRT
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
  #set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
else()
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions -fPIC")
  if (WIN32)
    # force static linking to gcc C++ runtime libraries
    set (CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ ${CMAKE_EXE_LINKER_FLAGS}")
  endif()
endif()
if (WIN32)
  add_definitions(-DUNICODE)

  # prevent implicit linkage with 'default' libraries
  set (CMAKE_CXX_STANDARD_LIBRARIES "")
  set (CMAKE_C_STANDARD_LIBRARIES "")
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

set (USED_SRCFILES "wglinfo.cpp")
set (USED_RCFILE "")
set (USED_MANFILES "")
if (WIN32)
  set (USED_RCFILE "wglinfo.rc")
  set (USED_MANFILES "wglinfo.manifest")
endif()

# main project target
add_executable (${PROJECT_NAME}
  ${USED_SRCFILES} ${USED_RCFILE} ${USED_MANFILES}
)
if (WIN32)
  target_link_libraries (${PROJECT_NAME} PRIVATE gdi32 user32)
endif()
target_link_libraries (${PROJECT_NAME} PRIVATE ${OPENGL_LIBRARIES})

# force static linking to winpthreads
if (WIN32 AND CMAKE_COMPILER_IS_GNUCXX)
  target_link_libraries (${PROJECT_NAME} PRIVATE -static gcc stdc++ winpthread -dynamic)
endif()
