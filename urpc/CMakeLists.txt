
add_library (urpc SHARED
             urpc-common.c
             urpc-data.c
             urpc-client.c
             urpc-udp-client.c
             urpc-tcp-client.c
             urpc-shm-client.c
             urpc-server.c
             urpc-udp-server.c
             urpc-tcp-server.c
             urpc-shm-server.c
             urpc-hash-table.c
             urpc-mem-chunk.c
             urpc-network.c
             urpc-${PLATFORM}-network.c
             urpc-${PLATFORM}-timer.c
             urpc-${PLATFORM}-mutex.c
             urpc-${PLATFORM}-rwmutex.c
             urpc-${PLATFORM}-semaphore.c
             urpc-${PLATFORM}-thread.c
             urpc-${PLATFORM}-shm.c)

if (WIN32)
  target_link_libraries (urpc wsock32 ws2_32)
endif ()

if (UNIX)
  target_link_libraries (urpc rt)
endif ()

install (TARGETS urpc EXPORT urpc
         RUNTIME DESTINATION bin
         LIBRARY DESTINATION lib
         ARCHIVE DESTINATION lib
         PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)

install (FILES urpc-exports.h
               urpc-client.h
               urpc-server.h
               urpc-data.h
               urpc-hash-table.h
               urpc-mem-chunk.h
               urpc-mutex.h
               urpc-network.h
               urpc-rwmutex.h
               urpc-semaphore.h
               urpc-shm.h
               urpc-thread.h
               urpc-timer.h
               urpc-types.h
         DESTINATION include/urpc
         PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
