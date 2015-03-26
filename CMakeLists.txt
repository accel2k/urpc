cmake_minimum_required( VERSION 2.6 )

project( uRPC )

include( FindPkgConfig )

if( NOT CMAKE_BUILD_TYPE )
  set( CMAKE_BUILD_TYPE Release )
endif()

#
# Compilers options.
if( ${CMAKE_C_COMPILER_ID} STREQUAL GNU  )
  set( CMAKE_C_FLAGS_RELEASE "-O2 -Wall" )
  set( CMAKE_C_FLAGS_DEBUG "-g -Wall" )
elseif( ${CMAKE_C_COMPILER_ID} STREQUAL MSVC )
  set( CMAKE_C_FLAGS_RELEASE "/O2" )
  set( CMAKE_C_FLAGS_DEBUG "/DEBUG" )
else()
  message( FATAL_ERROR "Unsupported compiler" )
endif()

# Additional tuning for GCC on UNIX platform
if( ${CMAKE_C_COMPILER_ID} STREQUAL GNU AND UNIX )
  set( CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -pthread" )
  set( CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -pthread" )
  set( PLATFORM "posix" )
endif()

# Additional tuning for GCC on Windows platform
if( ${CMAKE_C_COMPILER_ID} STREQUAL GNU AND WIN32 )
  set( CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -mthreads" )
  set( CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -mthreads" )
  set( PLATFORM "win32" )
endif()

# Additional tuning for MSVC on Windows platform
if( ${CMAKE_C_COMPILER_ID} STREQUAL MSVC AND WIN32 )
  set( PLATFORM "win32" )
endif()

#
# Output directories.
if( NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY )
  set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin )
endif()

if( NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY )
  set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin )
endif()

#
# Include directories.
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/urpc )

#
# Target.
add_subdirectory( urpc )
add_subdirectory( tests )
