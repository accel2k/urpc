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
#include "urpc-semaphore.h"
#include "urpc-thread.h"


volatile int start = 0;
volatile int server_ready = 0;

int counts = 250000;


void* server_thread( void *data )
{

  int i;

  uRpcSem *sem_start = urpc_sem_create( "start", 0, 1 );
  uRpcSem *sem_stop = urpc_sem_create( "stop", 0, 1 );

  if( sem_start == NULL || sem_stop == NULL )
    { printf( "can't create semaphores\n" ); return NULL; }

  server_ready = 1;

  while( start == 0 );

  for( i = 0; i < counts; i++ )
    {
    urpc_sem_lock( sem_start );
    urpc_sem_unlock( sem_stop );
    }

  printf( "server thread final timed lock\n" );

  urpc_sem_timedlock( sem_start, 2.0 );

  printf( "server thread stopped after %d iterations\n", i );

  urpc_sem_destroy( sem_start );
  urpc_sem_destroy( sem_stop );

  return NULL;

}


void* client_thread( void *data )
{

  int i;

  uRpcSem *sem_start;
  uRpcSem *sem_stop;

  while( server_ready == 0 );

  sem_start = urpc_sem_open( "start" );
  sem_stop = urpc_sem_open( "stop" );

  if( sem_start == NULL || sem_stop == NULL )
    { printf( "can't open semaphores\n" ); return NULL; }

  while( start == 0 );

  for( i = 0; i < counts; i++ )
    {
    urpc_sem_unlock( sem_start );
    urpc_sem_lock( sem_stop );
    }

  printf( "client thread final timed lock\n" );

  urpc_sem_timedlock( sem_stop, 2.0 );

  printf( "client thread stopped after %d iterations\n", i );

  urpc_sem_destroy( sem_start );
  urpc_sem_destroy( sem_stop );

  return NULL;

}


int main( int argc, char **argv )
{

  uRpcThread *server;
  uRpcThread *client;

  server = urpc_thread_create( server_thread, NULL );
  client = urpc_thread_create( client_thread, NULL );

  start = 1;

  urpc_thread_destroy( server );
  urpc_thread_destroy( client );

  return 0;

}
