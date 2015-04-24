
add_library( urpc SHARED
             urpc-common.c
             urpc-data.c
             urpc-client.c
             urpc-server.c
             urpc-udp-client.c
             urpc-udp-server.c
             urpc-hash-table.c
             urpc-mem-chunk.c
             urpc-network.c
             urpc-${PLATFORM}-network.c
             urpc-${PLATFORM}-timer.c
             urpc-${PLATFORM}-mutex.c
             urpc-${PLATFORM}-rwmutex.c
             urpc-${PLATFORM}-semaphore.c
             urpc-${PLATFORM}-thread.c
             urpc-${PLATFORM}-shm.c )

if( WIN32 )
  target_link_libraries( urpc wsock32 ws2_32 )
endif()

if( UNIX )
  target_link_libraries( urpc rt )
endif()
