cmake_minimum_required (VERSION 2.6)

project (uRPC)
set (URPC_VERSION 0.99)

include (TestBigEndian)
include (CheckIncludeFile)

#
# Default build type.
if (NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE "Release")
endif ()

#
# CPU bitness.
if (CMAKE_SIZEOF_VOID_P EQUAL 4)
  set (CPU32 1)
  add_definitions (-DCPU32)
elseif (CMAKE_SIZEOF_VOID_P EQUAL 8)
  set (CPU64 1)
  add_definitions (-DCPU64)
else ()
  MATH (EXPR CPU_BITNESS 8*${CMAKE_SIZEOF_VOID_P})
  message (FATAL_ERROR "${CPU_BITNESS}bit is unsupported CPU bitness")
endif ()

#
# Endianness check.
test_big_endian (URPC_BIG_ENDIAN)
if (${URPC_BIG_ENDIAN})
  add_definitions (-DURPC_BIG_ENDIAN)
endif ()

#
# Compilers options.
if (${CMAKE_C_COMPILER_ID} STREQUAL "GNU" OR ${CMAKE_C_COMPILER_ID} STREQUAL "Clang")
  if (CPU_ARCH STREQUAL "x86" AND CPU32)
    set (COMPILER_FLAGS "-march=pentium4 -mtune=generic -mfpmath=sse -Wall")
  else ()
    set (COMPILER_FLAGS "-Wall")
  endif ()
  if (UNIX)
    set (COMPILER_FLAGS "${COMPILER_FLAGS} -pthread")
    set (PLATFORM "posix")
  endif ()
  if (WIN32)
    set (COMPILER_FLAGS "${COMPILER_FLAGS} -mthreads")
    set (PLATFORM "win32")
  endif ()
  set (COMPILER_FLAGS_RELEASE "${COMPILER_FLAGS} -O2 ${OMP_FLAGS}")
  set (COMPILER_FLAGS_DEBUG "${COMPILER_FLAGS} -Og ${OMP_FLAGS}")
  set (ADDON_LIBRARIES m)
elseif (${CMAKE_C_COMPILER_ID} STREQUAL "MSVC")
  set (COMPILER_FLAGS_RELEASE "")
  set (COMPILER_FLAGS_DEBUG "")
  add_definitions( -D_WIN32_WINNT=0x0600 )
  add_definitions( -D_CRT_SECURE_NO_WARNINGS )
  set( PLATFORM "win32" )
else ()
  message (FATAL_ERROR "Unsupported compiler ${CMAKE_C_COMPILER_ID}")
endif ()

set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${COMPILER_FLAGS_RELEASE}")
set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${COMPILER_FLAGS_DEBUG}")
set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${COMPILER_FLAGS_RELEASE}")
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${COMPILER_FLAGS_DEBUG}")

#
# Output directories.
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

#
# Include directories.
check_include_file (stdint.h HAVE_STDINT_H)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/urpc)
if (${CMAKE_C_COMPILER_ID} STREQUAL MSVC AND NOT HAVE_STDINT_H)
  include_directories (${CMAKE_CURRENT_SOURCE_DIR}/vs)
endif ()

#
# Target.
add_subdirectory (urpc)
add_subdirectory (tests)

#
# Pkgconfig support.
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/urpc/urpc.pc.in ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/urpc.pc @ONLY)
install (FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/urpc.pc DESTINATION lib/pkgconfig PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
