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

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>


uRpcRWMutex *urpc_rwmutex_create( void )
{

  uRpcRWMutex *rwmutex = malloc( sizeof( uRpcRWMutex ) );

  if( rwmutex == NULL ) return NULL;
  if( pthread_rwlock_init( (pthread_rwlock_t*)rwmutex, NULL ) != 0 )
    { free( rwmutex ); return NULL; }

  return rwmutex;

}


void urpc_rwmutex_init( uRpcRWMutex *rwmutex )
{

  pthread_rwlock_init( (pthread_rwlock_t*)rwmutex, NULL );

}


void urpc_rwmutex_destroy( uRpcRWMutex *rwmutex )
{

  pthread_rwlock_destroy( (pthread_rwlock_t*)rwmutex );
  free( rwmutex );

}


void urpc_rwmutex_reader_lock( uRpcRWMutex *rwmutex )
{

  while( pthread_rwlock_rdlock( (pthread_rwlock_t*)rwmutex ) != 0 );

}


int urpc_rwmutex_reader_trylock( uRpcRWMutex *rwmutex )
{

  return pthread_rwlock_tryrdlock( (pthread_rwlock_t*)rwmutex ) == 0 ? 0 : 1;

}


void urpc_rwmutex_reader_unlock( uRpcRWMutex *rwmutex )
{

  while( pthread_rwlock_unlock( (pthread_rwlock_t*)rwmutex ) != 0 );

}


void urpc_rwmutex_writer_lock( uRpcRWMutex *rwmutex )
{

  while( pthread_rwlock_wrlock( (pthread_rwlock_t*)rwmutex ) != 0 );

}


int urpc_rwmutex_writer_trylock( uRpcRWMutex *rwmutex )
{

  return pthread_rwlock_trywrlock( (pthread_rwlock_t*)rwmutex ) == 0 ? 0 : 1;

}


void urpc_rwmutex_writer_unlock( uRpcRWMutex *rwmutex )
{

  while( pthread_rwlock_unlock( (pthread_rwlock_t*)rwmutex ) != 0 );

}
