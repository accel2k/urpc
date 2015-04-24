/*
 * uRpc - rpc (remote procedure call) library.
 *
 * Copyright 2009, 2010, 2014, 2015 Andrei Fadeev
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
 * \file urpc-client.h
 *
 * \author Andrei Fadeev
 * \date 20.03.2009
 * \brief
 *
 *
*/

#ifndef _urpc_client_h
#define _urpc_client_h

#include <urpc-exports.h>
#include <urpc-types.h>
#include <urpc-data.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct uRpcClient uRpcClient;


URPC_EXPORT uRpcClient *urpc_client_create( const char *uri, uint32_t max_data_size, double exec_timeout );


URPC_EXPORT void urpc_client_destroy( uRpcClient *urpc_client );


URPC_EXPORT int urpc_client_set_security( uRpcClient *urpc_client, uRpcSecurity mode );


URPC_EXPORT int urpc_client_set_priv_key( uRpcClient *urpc_client, const unsigned char *key );


URPC_EXPORT int urpc_client_set_pub_key( uRpcClient *urpc_client, const unsigned char *key, void *key_data );


URPC_EXPORT int urpc_client_connect( uRpcClient *urpc_client );


URPC_EXPORT uRpcData *urpc_client_lock( uRpcClient *urpc_client );


URPC_EXPORT uint32_t urpc_client_exec( uRpcClient *urpc_client, uint32_t proc_id );


URPC_EXPORT void urpc_client_unlock( uRpcClient *urpc_client );


#ifdef __cplusplus
} // extern "C"
#endif

#endif // _urpc_client_h
