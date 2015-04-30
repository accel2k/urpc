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
 * \file urpc-shm-server.h
 *
 * \author Andrei Fadeev
 * \date 12.02.2014
 * \brief
 *
 *
*/


#ifndef _urpc_shm_server_h
#define _urpc_shm_server_h

#include <urpc-types.h>
#include <urpc-data.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct uRpcSHMServer uRpcSHMServer;


uRpcSHMServer *urpc_shm_server_create( const char *uri, uint32_t threads_num, uint32_t max_data_size );


void urpc_shm_server_destroy( uRpcSHMServer *urpc_shm_server );


uRpcData *urpc_shm_server_recv( uRpcSHMServer *urpc_shm_server, uint32_t thread_id );


int urpc_shm_server_send( uRpcSHMServer *urpc_shm_server, uint32_t thread_id );


#ifdef __cplusplus

} // extern "C"
#endif

#endif // _urpc_shm_server_h
