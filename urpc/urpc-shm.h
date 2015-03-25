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

#ifndef _urpc_shm_h
#define _urpc_shm_h

#ifdef __cplusplus
extern "C" {
#endif


typedef struct uRpcShm uRpcShm;


uRpcShm *urpc_shm_create( const char *name, unsigned long size );
uRpcShm *urpc_shm_open( const char *name, unsigned long size );
uRpcShm *urpc_shm_open_ro( const char *name, unsigned long size );
void urpc_shm_destroy( uRpcShm *shm );

void *urpc_shm_map( uRpcShm *shm );
void urpc_shm_unmap( uRpcShm *shm );

void urpc_shm_remove( const char *name );


#ifdef __cplusplus
} // extern "C"
#endif

#endif // _urpc_shm_h
