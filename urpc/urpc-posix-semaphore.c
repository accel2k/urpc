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

#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <semaphore.h>


typedef struct uRpcSem {

  sem_t *sem;
  char *name;

} uRpcSem;


static uRpcSem *urpc_sem_create_int( const char *name, int initial_value, int create )
{

  uRpcSem *sem = malloc( sizeof( uRpcSem ) );
  int oflags = 0;

  if( create ) oflags = O_RDWR | O_CREAT | O_EXCL;
  else oflags = O_RDWR;

  if( sem == NULL )return NULL;
  sem->sem = sem_open( name, oflags, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP, initial_value );
  if( sem->sem == SEM_FAILED )
    { free( sem ); return NULL; }

  if( create )
    {
    int name_size = strlen( name ) + 1;
    sem->name = malloc( name_size );
    memcpy( sem->name, name, name_size );
    }
  else
    sem->name = NULL;

  return sem;

}


uRpcSem *urpc_sem_create( const char *name, uRpcSemStat stat )
{

  return urpc_sem_create_int( name, stat == URPC_SEM_LOCKED ? 0 : 1, 1 );

}


uRpcSem *urpc_sem_open( const char *name )
{

  return urpc_sem_create_int( name, 0, 0 );

}


void urpc_sem_destroy( uRpcSem *sem )
{

  sem_close( sem->sem );
  if( sem->name != NULL ) sem_unlink( sem->name );
  free( sem->name );
  free( sem );

}


void urpc_sem_lock( uRpcSem *sem )
{

  while( sem_wait( sem->sem ) != 0 );

}


int urpc_sem_trylock( uRpcSem *sem )
{

  return sem_trywait( sem->sem ) == 0 ? 1 : 0;

}


int urpc_sem_timedlock( uRpcSem *sem, double time )
{

  struct timeval cur_time;
  struct timespec sem_wait_time;

  gettimeofday( &cur_time, NULL );
  sem_wait_time.tv_sec = (int)time + cur_time.tv_sec;
  sem_wait_time.tv_nsec = 1000000000 * ( time - (int)time ) + 1000 * cur_time.tv_usec;
  if( sem_wait_time.tv_nsec >= 1000000000 )
    {
    sem_wait_time.tv_nsec -= 1000000000;
    sem_wait_time.tv_sec += 1;
    }

  while( 1 )
    {
    if( sem_timedwait( sem->sem, &sem_wait_time ) < 0 )
      {
      if( errno == EINTR ) continue;
      else if( errno == ETIMEDOUT ) return 1;
      else return -1;
      }
    break;
    }

  return 0;

}


void urpc_sem_unlock( uRpcSem *sem )
{

  while( sem_post( sem->sem ) != 0 );

}


void urpc_sem_remove( const char *name )
{

  sem_unlink( name );

}
