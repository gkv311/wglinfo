# Dummy project for generating libEGL.lib and libGLESv2.lib via MSVC
# for linking against DLLs built by MinGW.

cmake_minimum_required (VERSION 3.5)

project (libEGLDummy)

set (CMAKE_CXX_STANDARD 11)
if (MSVC)
  # static linking with CRT
  set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
  #set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
endif()
if (WIN32)
  # prevent implicit linkage with 'default' libraries
  set (CMAKE_CXX_STANDARD_LIBRARIES "")
  set (CMAKE_C_STANDARD_LIBRARIES "")
endif()

include_directories ("${CMAKE_CURRENT_SOURCE_DIR}/include")
add_library (libEGL    SHARED libEGLdummy.cpp)
add_library (libGLESv2 SHARED libGLESv2dummy.cpp)
