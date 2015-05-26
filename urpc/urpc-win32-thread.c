/*
 * uRPC - rpc (remote procedure call) library.
 *
 * Copyright 2015 Andrei Fadeev (andrei@webcontrol.ru)
 *
 * This file is part of uRPC.
 *
 * uRPC is dual-licensed: you can redistribute it and/or modify
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
 * Alternatively, you can license this code under a commercial license.
 * Contact the author in this case.
 *
*/

#include "urpc-thread.h"

#include <windows.h>


uRpcThread *urpc_thread_create( urpc_thread_func func, void *data )
{

  uRpcThread *thread = malloc( sizeof( uRpcThread ) );

  if( thread == NULL ) return NULL;
  *thread = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)func, data, 0, NULL );
  if( *thread == NULL )
    { free( thread ); return NULL; }

  return thread;

}


void urpc_thread_destroy( uRpcThread *thread )
{

  while( WaitForSingleObject( *thread, INFINITE ) != WAIT_OBJECT_0 );

  free( thread );

}


void *urpc_thread_join( uRpcThread *thread )
{

  while( WaitForSingleObject( *thread, INFINITE ) != WAIT_OBJECT_0 );

  return NULL;

}

