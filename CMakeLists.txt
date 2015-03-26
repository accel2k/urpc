cmake_minimum_required( VERSION 2.6 )

project( uRPC )

include( FindPkgConfig )

if( NOT CMAKE_BUILD_TYPE )
  set( CMAKE_BUILD_TYPE Release )
endif( NOT CMAKE_BUILD_TYPE )

#
# Compilers options.
if( ${CMAKE_C_COMPILER_ID} EQUAL GNU  )
  set( CMAKE_C_FLAGS_RELEASE "-O2 -Wall" )
  set( CMAKE_C_FLAGS_DEBUG "-g -Wall" )
elseif( ${CMAKE_C_COMPILER_ID} EQUAL MSVC )
  set( CMAKE_C_FLAGS_RELEASE "/O2" )
  set( CMAKE_C_FLAGS_DEBUG "/DEBUG" )
else( ${CMAKE_C_COMPILER_ID} EQUAL GNU  )
  message( FATAL_ERROR "Unsupported compiler" )
endif( ${CMAKE_C_COMPILER_ID} EQUAL GNU  )

# Additional tuning for GCC on UNIX platform
if( ${CMAKE_C_COMPILER_ID} EQUAL GNU AND UNIX )
  set( CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -pthread" )
  set( CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -pthread" )
  set( PLATFORM "posix" )
endif( ${CMAKE_C_COMPILER_ID} EQUAL GNU AND UNIX )

# Additional tuning for GCC on Windows platform
if( ${CMAKE_C_COMPILER_ID} EQUAL GNU AND WIN32 )
  set( CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -mthreads" )
  set( CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -mthreads" )
  set( PLATFORM "win32" )
endif( ${CMAKE_C_COMPILER_ID} EQUAL GNU AND WIN32 )

# Additional tuning for MSVC on Windows platform
if( ${CMAKE_C_COMPILER_ID} EQUAL MSVC AND WIN32 )
  set( PLATFORM "win32" )
endif( ${CMAKE_C_COMPILER_ID} EQUAL MSVC AND WIN32 )

#
# Output directories.
if( NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY )
  set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin )
endif( NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY )

if( NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY )
  set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin )
endif( NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY )

#
# Include directories.
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/urpc )

#
# Target.
add_subdirectory( urpc )
add_subdirectory( tests )
