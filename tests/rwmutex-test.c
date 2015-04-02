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
#include "urpc-rwmutex.h"
#include "urpc-thread.h"


volatile int start = 0;

uRpcRWMutex *mutex;

int counts = 2500000;
int cur_i, prev_i;


void* reader_thread_func( void *data )
{

  int id = *(int*)data;

  while( start == 0 );

  while( cur_i < counts )
    {
    urpc_rwmutex_reader_lock( mutex );
    if( prev_i != cur_i - 1 ) printf( "index error %d <=> %d\n", prev_i, cur_i );
    urpc_rwmutex_reader_unlock( mutex );
    }

  printf( "reader thread %d stopped after %d iterations\n", id, cur_i );

  return NULL;

}


void* writer_thread_func( void *data )
{

  int id = *(int*)data;

  while( start == 0 );

  while( cur_i < counts )
    {
    urpc_rwmutex_writer_lock( mutex );
    prev_i = cur_i;
    cur_i += 1;
    urpc_rwmutex_writer_unlock( mutex );
    }

  printf( "writer thread %d stopped after %d iterations\n", id, cur_i );

  return NULL;

}


int main( int argc, char **argv )
{

  int id1 = 1;
  int id2 = 2;
  int id3 = 3;

  uRpcThread *rthread1;
  uRpcThread *rthread2;
  uRpcThread *wthread;

  prev_i = 0;
  cur_i = 1;

  rthread1 = urpc_thread_create( reader_thread_func, &id1 );
  rthread2 = urpc_thread_create( reader_thread_func, &id2 );
  wthread = urpc_thread_create( writer_thread_func, &id3 );
  mutex = urpc_rwmutex_create();

  start = 1;

  urpc_thread_destroy( rthread1 );
  urpc_thread_destroy( rthread2 );
  urpc_thread_destroy( wthread );
  urpc_rwmutex_destroy( mutex );

  return 0;

}
