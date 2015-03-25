
add_library( urpc SHARED
             urpc-common.c
             urpc-${PLATFORM}-network.c
             urpc-${PLATFORM}-mutex.c
             urpc-${PLATFORM}-semaphore.c
             urpc-${PLATFORM}-thread.c
             urpc-${PLATFORM}-shm.c )

if( WIN32 )
  add_definitions( -D_WIN32_WINNT=0x0501 )
  target_link_libraries( urpc wsock32 )
endif( WIN32 )

if( UNIX )
  target_link_libraries( urpc rt )
endif( UNIX )
