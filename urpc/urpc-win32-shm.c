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

#include <windows.h>


typedef struct uRpcShm {

  HANDLE shm;
  int ro;
  void *maddr;

} uRpcShm;


uRpcShm *urpc_shm_create( const char *name, unsigned long size )
{

  uRpcShm *shm = malloc( sizeof( uRpcShm ) );
  if( shm == NULL ) return NULL;

  shm->shm = CreateFileMapping( INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, size, name );
  if( shm->shm == NULL )
    { free( shm ); return NULL; }

  shm->ro = 0;
  shm->maddr = NULL;

  return shm;

}


uRpcShm *urpc_shm_open( const char *name, unsigned long size )
{

  uRpcShm *shm = malloc( sizeof( uRpcShm ) );
  if( shm == NULL ) return NULL;

  shm->shm = OpenFileMapping( FILE_MAP_WRITE, FALSE, name );
  if( shm->shm == NULL )
    { free( shm ); return NULL; }

  shm->ro = 0;
  shm->maddr = NULL;

  return shm;

}

uRpcShm *urpc_shm_open_ro( const char *name, unsigned long size )
{

  uRpcShm *shm = malloc( sizeof( uRpcShm ) );
  if( shm == NULL ) return NULL;

  shm->shm = OpenFileMapping( FILE_MAP_READ, FALSE, name );
  if( shm->shm == NULL )
    { free( shm ); return NULL; }

  shm->ro = 1;
  shm->maddr = NULL;

  return shm;

}


void urpc_shm_destroy( uRpcShm *shm )
{

  if( shm->maddr != NULL ) UnmapViewOfFile( shm->maddr );
  CloseHandle( shm->shm );

}


void urpc_shm_remove( const char *name )
{
}


void *urpc_shm_map( uRpcShm *shm )
{

  DWORD pflags = shm->ro ? FILE_MAP_READ : FILE_MAP_WRITE;

  shm->maddr = MapViewOfFile( shm->shm, pflags, 0, 0, 0 );
  return shm->maddr;

}
