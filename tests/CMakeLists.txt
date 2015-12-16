add_executable (data-test data-test.c)
add_executable (common-test common-test.c)
add_executable (hash-table-test hash-table-test.c)
add_executable (mem-chunk-test mem-chunk-test.c)
add_executable (network-test network-test.c)
add_executable (timer-test timer-test.c)
add_executable (mutex-test mutex-test.c)
add_executable (rwmutex-test rwmutex-test.c)
add_executable (semaphore-test semaphore-test.c)
add_executable (shm-server-test shm-server-test.c)
add_executable (shm-client-test shm-client-test.c)
add_executable (urpc-test urpc-test.c)

target_link_libraries (data-test urpc)
target_link_libraries (common-test urpc)
target_link_libraries (hash-table-test urpc)
target_link_libraries (mem-chunk-test urpc)
target_link_libraries (network-test urpc)
target_link_libraries (timer-test urpc)
target_link_libraries (mutex-test urpc)
target_link_libraries (rwmutex-test urpc)
target_link_libraries (semaphore-test urpc)
target_link_libraries (shm-server-test urpc)
target_link_libraries (shm-client-test urpc)
target_link_libraries (urpc-test urpc)

if (WIN32)
  target_link_libraries (common-test wsock32 ws2_32)
  target_link_libraries (network-test wsock32 ws2_32)
  target_link_libraries (timer-test wsock32 winmm)
endif ()

install (TARGETS urpc-test EXPORT urpc-test
         RUNTIME DESTINATION bin
         LIBRARY DESTINATION lib
         ARCHIVE DESTINATION lib
         PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
