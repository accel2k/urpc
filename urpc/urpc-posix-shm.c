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

#include "urpc-shm.h"

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>


typedef struct uRpcShm {

  int shm_id;
  unsigned long size;
  int ro;
  char *name;
  void *maddr;

} uRpcShm;


static uRpcShm *urpc_shm_create_int( const char *name, unsigned long size, int create, int ro )
{

  uRpcShm *shm = malloc( sizeof( uRpcShm ) );
  int oflags = 0;

  if( create ) oflags = O_RDWR | O_CREAT | O_EXCL;
  else if( !ro ) oflags = O_RDWR;
  else oflags = O_RDONLY;

  if( shm == NULL ) return NULL;
  shm->shm_id = shm_open( name, oflags, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP );
  if( shm->shm_id < 0 )
    { free( shm ); return NULL; }

  if( create )
    {
    int name_size = strlen( name ) + 1;
    shm->name = malloc( name_size );
    memcpy( shm->name, name, name_size );
    }
  else
    shm->name = NULL;

  shm->ro = ro;
  shm->size = size;
  shm->maddr = NULL;

  return shm;

}


uRpcShm *urpc_shm_create( const char *name, unsigned long size )
{

  return urpc_shm_create_int( name, size, 1, 0 );

}


uRpcShm *urpc_shm_open( const char *name, unsigned long size )
{

  return urpc_shm_create_int( name, size, 0, 0 );

}


uRpcShm *urpc_shm_open_ro( const char *name, unsigned long size )
{

  return urpc_shm_create_int( name, size, 0, 1 );

}


void urpc_shm_destroy( uRpcShm *shm )
{

  if( shm->maddr != NULL ) munmap( shm->maddr, shm->size );
  if( shm->name != NULL ) shm_unlink( shm->name );
  free( shm->name );
  free( shm );

}


void urpc_shm_remove( const char *name )
{

  shm_unlink( name );

}


void *urpc_shm_map( uRpcShm *shm )
{

  int pflags = shm->ro ? PROT_READ : PROT_READ | PROT_WRITE;

  shm->maddr = mmap( NULL, shm->size, pflags, MAP_SHARED, shm->shm_id, 0 );
  if( shm->maddr == MAP_FAILED ) return NULL;

  if( ftruncate( shm->shm_id, shm->size ) < 0 )
    { munmap( shm->maddr, shm->size ); return NULL; }

  return shm->maddr;

}
