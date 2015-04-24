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

uRpcMutex mutex;

int counts = 2500000;


void* thread_func( void *data )
{

  int i = 0;
  int id = *(int*)data;

  while( start == 0 );

  while( i < counts )
    {
    urpc_mutex_lock( &mutex );
    i++;
    urpc_mutex_unlock( &mutex );
    }

  printf( "thread %d stopped after %d iterations\n", id, i );

  return NULL;

}


int main( int argc, char **argv )
{

  int id1 = 1;
  int id2 = 2;

  uRpcThread *thread1;
  uRpcThread *thread2;

  urpc_mutex_init( &mutex );
  thread1 = urpc_thread_create( thread_func, &id1 );
  thread2 = urpc_thread_create( thread_func, &id2 );

  start = 1;

  urpc_thread_destroy( thread1 );
  urpc_thread_destroy( thread2 );
  urpc_mutex_clear( &mutex );

  return 0;

}
