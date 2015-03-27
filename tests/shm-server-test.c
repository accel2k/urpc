/*
 * uRpc - rpc (remote procedure call) library.
 *
 * Copyright 2015 Andrei Fadeev
 *
 * This file is part of uRPC.
 *
 * uRPC is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * uRPC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
*/

#include <stdio.h>
#include "urpc-shm.h"
#include "urpc-semaphore.h"

#define SHM_NAME "urpc-server-test"
#define SEM_START_NAME "urpc-server-test-start"
#define SEM_STOP_NAME "urpc-server-test-stop"

int main( int argc, char **argv )
{

  uRpcShm *server_shm;
  uRpcSem *server_start_sem;
  uRpcSem *server_stop_sem;

  double *io;

  urpc_shm_remove( SHM_NAME );
  urpc_sem_remove( SEM_START_NAME );
  urpc_sem_remove( SEM_STOP_NAME );

  server_shm = urpc_shm_create( SHM_NAME, 2 * sizeof( double ) );
  server_start_sem = urpc_sem_create( SEM_START_NAME, URPC_SEM_LOCKED, 1 );
  server_stop_sem = urpc_sem_create( SEM_STOP_NAME, URPC_SEM_LOCKED, 1 );

  io = urpc_shm_map( server_shm );

  while( 1 )
    {

    urpc_sem_lock( server_start_sem );
    io[1] = 1.23456 * io[0];
    urpc_sem_unlock( server_stop_sem );

    }

  return 0;

}
