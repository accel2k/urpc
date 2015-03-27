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

#include "urpc-semaphore.h"

#include <windows.h>


typedef struct uRpcSem {

  HANDLE sem;

} uRpcSem;


uRpcSem *urpc_sem_create( const char *name, uRpcSemStat stat, int queue )
{

  uRpcSem *sem = malloc( sizeof( uRpcSem ) );
  if( sem == NULL ) return NULL;

  sem->sem = CreateSemaphore( NULL, stat == URPC_SEM_LOCKED ? 0 : queue, queue, name );
  if( sem->sem == NULL )
    { free( sem ); return NULL; }

  return sem;

}


uRpcSem *urpc_sem_open( const char *name )
{

  uRpcSem *sem = malloc( sizeof( uRpcSem ) );
  if( sem == NULL ) return NULL;

  sem->sem = OpenSemaphore( SEMAPHORE_ALL_ACCESS, FALSE, name );
  if( sem->sem == NULL )
    { free( sem ); return NULL; }

  return sem;

}


void urpc_sem_destroy( uRpcSem *sem )
{

  CloseHandle( sem->sem );
  free( sem );

}


void urpc_sem_remove( const char *name )
{
}


void urpc_sem_lock( uRpcSem *sem )
{

  while( WaitForSingleObject( sem->sem, INFINITE ) != WAIT_OBJECT_0 );

}


int urpc_sem_trylock( uRpcSem *sem )
{

  return WaitForSingleObject( sem->sem, 0L ) == WAIT_OBJECT_0 ? 0 : -1;

}


int urpc_sem_timedlock( uRpcSem *sem, double time )
{

  DWORD wait_time = (DWORD)( 1000 * time );
  DWORD wait_stat = WaitForSingleObject( sem->sem, wait_time );

  if( wait_stat == WAIT_OBJECT_0 ) return 0;
  if( wait_stat == WAIT_TIMEOUT ) return 1;
  return -1;

}


void urpc_sem_unlock( uRpcSem *sem )
{

  while( !ReleaseSemaphore( sem->sem, 1,NULL ) );

}