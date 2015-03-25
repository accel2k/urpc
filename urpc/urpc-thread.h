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

#ifndef _urpc_thread_h
#define _urpc_thread_h

#ifdef __cplusplus
extern "C" {
#endif


typedef struct uRpcThread uRpcThread;

typedef void* (*urpc_thread_func)( void *data );


uRpcThread *urpc_thread_create( urpc_thread_func func, void *data );
void urpc_thread_destroy( uRpcThread *thread );

void *urpc_thread_join( uRpcThread *thread );
void urpc_thread_exit( void *retval );


#ifdef __cplusplus
} // extern "C"
#endif

#endif // _urpc_thread_h
