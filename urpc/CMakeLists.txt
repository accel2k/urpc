
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

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/urpc.pc.in
                ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/urpc.pc
                @ONLY)

install (TARGETS urpc
         COMPONENT runtime
         RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
         LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
         PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)

install (FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/urpc.pc
         COMPONENT development
         DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig"
         PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)

install (FILES urpc-exports.h
               urpc-client.h
               urpc-server.h
               urpc-data.h
               urpc-types.h
         COMPONENT development
         DESTINATION include/urpc
         PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
