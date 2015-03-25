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
#include "urpc-mutex.h"
#include "urpc-thread.h"


volatile int start = 0;

uRpcMutex *mutex_start;
uRpcMutex *mutex_stop;

int counts = 2500000;


void* server_thread( void *data )
{

  int i;

  while( start == 0 );

  for( i = 0; i < counts; i++ )
    {
    urpc_mutex_lock( mutex_start );
    urpc_mutex_unlock( mutex_stop );
    }

  printf( "server thread final timed lock\n" );

  urpc_mutex_timedlock( mutex_start, 2.0 );

  printf( "server thread stopped after %d iterations\n", i );

  return NULL;

}


void* client_thread( void *data )
{

  int i;

  while( start == 0 );

  for( i = 0; i < counts; i++ )
    {
    urpc_mutex_unlock( mutex_start );
    urpc_mutex_lock( mutex_stop );
    }

  printf( "client thread final timed lock\n" );

  urpc_mutex_timedlock( mutex_stop, 2.0 );

  printf( "client thread stopped after %d iterations\n", i );

  return NULL;

}


int main( int argc, char **argv )
{

  uRpcThread *server;
  uRpcThread *client;

  server = urpc_thread_create( server_thread, NULL );
  client = urpc_thread_create( client_thread, NULL );

  mutex_start = urpc_mutex_create();
  mutex_stop = urpc_mutex_create();

  urpc_mutex_lock( mutex_start );
  urpc_mutex_lock( mutex_stop );

  start = 1;

  urpc_thread_destroy( server );
  urpc_thread_destroy( client );

  urpc_mutex_destroy( mutex_start );
  urpc_mutex_destroy( mutex_stop );

  return 0;

}
