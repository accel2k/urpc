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

#include "urpc-mutex.h"

#include <windows.h>


typedef struct uRpcMutex {

  CRITICAL_SECTION mutex;

} uRpcMutex;



uRpcMutex *urpc_mutex_create( void )
{

  uRpcMutex *mutex = malloc( sizeof( uRpcMutex ) );

  if( mutex == NULL ) return NULL;
  InitializeCriticalSection( (LPCRITICAL_SECTION)mutex );
  return mutex;

}


void urpc_mutex_destroy( uRpcMutex *mutex )
{

  DeleteCriticalSection( (LPCRITICAL_SECTION)mutex );
  free( mutex );

}


void urpc_mutex_lock( uRpcMutex *mutex )
{

  EnterCriticalSection( (LPCRITICAL_SECTION)mutex );

}


int urpc_mutex_trylock( uRpcMutex *mutex )
{

  return TryEnterCriticalSection( (LPCRITICAL_SECTION)mutex ) ? 0 : -1;

}


void urpc_mutex_unlock( uRpcMutex *mutex )
{

  LeaveCriticalSection( (LPCRITICAL_SECTION)mutex );

}
