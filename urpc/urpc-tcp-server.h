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
 * \file urpc-tcp-server.h
 *
 * \author Andrei Fadeev
 * \date 12.02.2014
 * \brief
 *
 *
*/

#ifndef _urpc_tcp_server_h
#define _urpc_tcp_server_h

#include <urpc-network.h>
#include <urpc-types.h>
#include <urpc-data.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct uRpcTCPServer uRpcTCPServer;


uRpcTCPServer *urpc_tcp_server_create( const char *uri, uint32_t threads_num, uint32_t max_clients, uint32_t max_data_size, double timeout );


void urpc_tcp_server_destroy( uRpcTCPServer *urpc_tcp_server );


uRpcData *urpc_tcp_server_recv( uRpcTCPServer *urpc_tcp_server, uint32_t thread_id );


int urpc_tcp_server_send( uRpcTCPServer *urpc_tcp_server, uint32_t thread_id );


SOCKET urpc_tcp_server_get_client_socket( uRpcTCPServer *urpc_tcp_server, uint32_t thread_id );


int urpc_tcp_server_disconnect_client( uRpcTCPServer *urpc_tcp_server, SOCKET wsocket );


#ifdef __cplusplus
} // extern "C"
#endif

#endif // _urpc_tcp_server_h
