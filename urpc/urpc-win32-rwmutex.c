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

#include "urpc-rwmutex.h"

#include <windows.h>


uRpcRWMutex *urpc_rwmutex_create( void )
{

  uRpcRWMutex *rwmutex = malloc( sizeof( SRWLOCK ) );

  if( rwmutex == NULL ) return NULL;
  InitializeSRWLock( (PSRWLOCK)rwmutex );

  return rwmutex;

}


void urpc_rwmutex_init( uRpcRWMutex *rwmutex )
{

  InitializeSRWLock( (PSRWLOCK)rwmutex );

}


void urpc_rwmutex_destroy( uRpcRWMutex *rwmutex )
{

  free( rwmutex );

}


void urpc_rwmutex_reader_lock( uRpcRWMutex *rwmutex )
{

  AcquireSRWLockShared( (PSRWLOCK)rwmutex );

}


int urpc_rwmutex_reader_trylock( uRpcRWMutex *rwmutex )
{

  return TryAcquireSRWLockShared( (PSRWLOCK)rwmutex ) != 0 ? 0 : 1;

}


void urpc_rwmutex_reader_unlock( uRpcRWMutex *rwmutex )
{

  ReleaseSRWLockShared( (PSRWLOCK)rwmutex );

}


void urpc_rwmutex_writer_lock( uRpcRWMutex *rwmutex )
{

  AcquireSRWLockExclusive( (PSRWLOCK)rwmutex );

}


int urpc_rwmutex_writer_trylock( uRpcRWMutex *rwmutex )
{

  return TryAcquireSRWLockExclusive( (PSRWLOCK)rwmutex ) != 0 ? 0 : 1;

}


void urpc_rwmutex_writer_unlock( uRpcRWMutex *rwmutex )
{

  ReleaseSRWLockExclusive( (PSRWLOCK)rwmutex );

}
