
cmake_minimum_required (VERSION 3.0)

project (uRPC)

enable_testing ()

include (TestBigEndian)
include (CheckIncludeFile)
include (GNUInstallDirs)

set (URPC_VERSION 1.0)

if (NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE Release)
endif ()

if (NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
  set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
  set (CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/bin")
  set (CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/bin")
endif ()

if (NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY)
  set (CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
  set (CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/bin")
  set (CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/bin")
endif ()

if (NOT CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
  set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
  set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/bin")
  set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/bin")
endif ()

test_big_endian (URPC_BIG_ENDIAN)
if (${URPC_BIG_ENDIAN})
  add_definitions (-DURPC_BIG_ENDIAN)
endif ()

if (${CMAKE_C_COMPILER_ID} STREQUAL GNU)
  set (DEBUG_OPT_LEVEL "-Og")
elseif (${CMAKE_C_COMPILER_ID} STREQUAL Clang)
  set (DEBUG_OPT_LEVEL "-O1")
endif ()

if (UNIX)
  set (THREAD_C_FLAGS " -pthread")
  set (PLATFORM posix)
endif ()
if (WIN32)
  add_definitions (-D_WIN32_WINNT=0x0600)
  set (THREAD_C_FLAGS " -mthreads")
  set (PLATFORM win32)
endif ()

if (${CMAKE_C_COMPILER_ID} STREQUAL GNU OR ${CMAKE_C_COMPILER_ID} STREQUAL Clang)
  set (C_FLAGS "-Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers ${THREAD_C_FLAGS} $ENV{CFLAGS}")
  set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O2 ${C_FLAGS}")
  set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${DEBUG_OPT_LEVEL} ${C_FLAGS}")
elseif (${CMAKE_C_COMPILER_ID} STREQUAL MSVC)
  set (C_FLAGS "/wd4244 $ENV{CFLAGS}")
  set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${C_FLAGS}")
  set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${C_FLAGS}")
  add_definitions (-D_CRT_SECURE_NO_WARNINGS)
  add_definitions (-D_MSVC_COMPILER)
  set (PLATFORM win32)
else ()
  message (FATAL_ERROR "Unsupported compiler ${CMAKE_C_COMPILER_ID}")
endif ()

check_include_file (stdint.h HAVE_STDINT_H)
include_directories ("${CMAKE_CURRENT_SOURCE_DIR}/urpc")
if (${CMAKE_C_COMPILER_ID} STREQUAL MSVC AND NOT HAVE_STDINT_H)
  include_directories ("${CMAKE_CURRENT_SOURCE_DIR}/vs")
endif ()

add_subdirectory (urpc)
add_subdirectory (tests)
