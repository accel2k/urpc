
cmake_minimum_required (VERSION 2.6)

project (uRPC)
set (URPC_VERSION 0.99)

include (TestBigEndian)
include (CheckIncludeFile)

if (NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE "Release")
endif ()

test_big_endian (URPC_BIG_ENDIAN)
if (${URPC_BIG_ENDIAN})
  add_definitions (-DURPC_BIG_ENDIAN)
endif ()

if (${CMAKE_C_COMPILER_ID} STREQUAL "GNU" OR ${CMAKE_C_COMPILER_ID} STREQUAL "Clang")
  set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O2 -Wall -Wextra -Wno-unused-parameter $ENV{CFLAGS}")
  set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O1 -Wall -Wextra -Wno-unused-parameter $ENV{CFLAGS}")
  if (UNIX)
    set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}  -pthread")
    set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}  -pthread")
    set (PLATFORM "posix")
  endif ()
  if (WIN32)
    add_definitions (-D_WIN32_WINNT=0x0600)
    set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}  -mthreads")
    set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}  -mthreads")
    set (PLATFORM "win32")
  endif ()
elseif (${CMAKE_C_COMPILER_ID} STREQUAL "MSVC")
  set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /wd4244 $ENV{CFLAGS}")
  set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /wd4244 $ENV{CFLAGS}")
  add_definitions (-D_WIN32_WINNT=0x0600)
  add_definitions (-D_CRT_SECURE_NO_WARNINGS)
  add_definitions (-D_MSVC_COMPILER)
  set (PLATFORM "win32")
else ()
  message (FATAL_ERROR "Unsupported compiler ${CMAKE_C_COMPILER_ID}")
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

check_include_file (stdint.h HAVE_STDINT_H)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/urpc)
if (${CMAKE_C_COMPILER_ID} STREQUAL MSVC AND NOT HAVE_STDINT_H)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/vs)
endif ()

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/urpc/urpc.pc.in
                ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/urpc.pc
                @ONLY)
install (FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/urpc.pc
         DESTINATION lib/pkgconfig
         PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)

add_subdirectory (urpc)
add_subdirectory (tests)
