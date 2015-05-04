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
 * \file urpc-tcp-client.h
 *
 * \brief
 * \author Andrei Fadeev (andrei@webcontrol.ru)
 * \date 2009, 2010, 2014, 2015
 * \copyright GNU General Public License version 3 or later
 *
 *
*/

#ifndef _urpc_tcp_client_h
#define _urpc_tcp_client_h

#include <urpc-types.h>
#include <urpc-data.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct uRpcTCPClient uRpcTCPClient;


uRpcTCPClient *urpc_tcp_client_create( const char *uri, uint32_t max_data_size, double exec_timeout );


void urpc_tcp_client_destroy( uRpcTCPClient *urpc_tcp_client );


int urpc_tcp_client_connect( uRpcTCPClient *urpc_tcp_client );


uRpcData *urpc_tcp_client_lock( uRpcTCPClient *urpc_tcp_client );


uint32_t urpc_tcp_client_exchange( uRpcTCPClient *urpc_tcp_client );


#ifdef __cplusplus
} // extern "C"
#endif

#endif // _urpc_tcp_client_h
