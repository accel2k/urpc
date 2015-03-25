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

#ifndef _urpc_mutex_h
#define _urpc_mutex_h

#ifdef __cplusplus
extern "C" {
#endif


typedef struct uRpcMutex uRpcMutex;


uRpcMutex *urpc_mutex_create( void );
void urpc_mutex_destroy( uRpcMutex *mutex );

void urpc_mutex_lock( uRpcMutex *mutex );
int urpc_mutex_trylock( uRpcMutex *mutex );
int urpc_mutex_timedlock( uRpcMutex *mutex, double time );
void urpc_mutex_unlock( uRpcMutex *mutex );


#ifdef __cplusplus
} // extern "C"
#endif

#endif // _urpc_mutex_h
