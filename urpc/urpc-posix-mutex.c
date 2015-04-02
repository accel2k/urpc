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

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>


uRpcMutex *urpc_mutex_create( void )
{

  uRpcMutex *mutex = malloc( sizeof( uRpcMutex ) );

  if( mutex == NULL ) return NULL;
  if( pthread_mutex_init( (pthread_mutex_t*)mutex, NULL ) != 0 )
    { free( mutex ); return NULL; }

  return mutex;

}


void urpc_mutex_init( uRpcMutex *mutex )
{

  pthread_mutex_init( (pthread_mutex_t*)mutex, NULL );

}


void urpc_mutex_destroy( uRpcMutex *mutex )
{

  pthread_mutex_destroy( (pthread_mutex_t*)mutex );
  free( mutex );

}


void urpc_mutex_lock( uRpcMutex *mutex )
{

  while( pthread_mutex_lock( (pthread_mutex_t*)mutex ) != 0 );

}


int urpc_mutex_trylock( uRpcMutex *mutex )
{

  return pthread_mutex_trylock( (pthread_mutex_t*)mutex ) == 0 ? 0 : 1;

}


void urpc_mutex_unlock( uRpcMutex *mutex )
{

  while( pthread_mutex_unlock( (pthread_mutex_t*)mutex ) != 0 );

}
