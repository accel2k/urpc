/*
 * uRpc - rpc (remote procedure call) library.
 *
 * Copyright 2014, 2015 Andrei Fadeev
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

/*!
 * \file urpc-server.h
 *
 * \author Andrei Fadeev
 * \date 12.02.2014
 * \brief
 *
 *
*/

#ifndef _urpc_server_h
#define _urpc_server_h

#include <urpc-exports.h>
#include <urpc-types.h>
#include <urpc-data.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct uRpcServer uRpcServer;


typedef int (*urpc_server_callback)( uint32_t session, uRpcData *urpc_data, void *proc_data, void *key_data );


URPC_EXPORT uRpcServer *urpc_server_create( const char *uri, uint32_t max_data_size, double data_timeout, uint32_t threads_num, uint32_t max_clients );


URPC_EXPORT void urpc_server_destroy( uRpcServer *urpc_server );


URPC_EXPORT int urpc_server_set_security( uRpcServer *urpc_server, uRpcSecurity mode );


URPC_EXPORT int urpc_server_set_priv_key( uRpcServer *urpc_server, const unsigned char *key );


URPC_EXPORT int urpc_server_add_pub_key( uRpcServer *urpc_server, const unsigned char *key, void *key_data );


URPC_EXPORT int urpc_server_add_proc( uRpcServer *urpc_server, uint32_t proc_id, urpc_server_callback proc, void *proc_data );


URPC_EXPORT int urpc_server_bind( uRpcServer *urpc_server );


#ifdef __cplusplus
} // extern "C"
#endif

#endif // _urpc_server_h
