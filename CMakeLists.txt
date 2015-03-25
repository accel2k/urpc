cmake_minimum_required( VERSION 2.6 )

project( uRPC )

include( FindPkgConfig )

if( NOT CMAKE_BUILD_TYPE )
  set( CMAKE_BUILD_TYPE Release )
endif( NOT CMAKE_BUILD_TYPE )

#
# Compilers options.
if( CMAKE_COMPILER_IS_GNUCC )
  set( CMAKE_C_FLAGS_RELEASE "-O2 -Wall" )
  set( CMAKE_C_FLAGS_DEBUG "-g -Wall" )
else( CMAKE_COMPILER_IS_GNUCC )
  message( FATAL_ERROR "Unsupported compiler" )
endif( CMAKE_COMPILER_IS_GNUCC )

# Additional tuning for UNIX platform
if( CMAKE_COMPILER_IS_GNUCC AND UNIX )
  set( CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -pthread" )
  set( CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -pthread" )
  set( PLATFORM "posix" )
endif( CMAKE_COMPILER_IS_GNUCC AND UNIX )

# Additional tuning for Windows platform
if( CMAKE_COMPILER_IS_GNUCC AND WIN32 )
  set( CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -mthreads" )
  set( CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -mthreads" )
  set( PLATFORM "win32" )
endif( CMAKE_COMPILER_IS_GNUCC AND WIN32 )

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
