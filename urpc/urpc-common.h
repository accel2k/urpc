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

#ifndef _urpc_common_h
#define _urpc_common_h

#include <urpc-exports.h>
#include <urpc-network.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef enum { URPC_TRANSPORT_UNKNOWN = 0, URPC_TRANSPORT_UDP, URPC_TRANSPORT_TCP, URPC_TRANSPORT_SHM } uRpcTransportType;


URPC_EXPORT uRpcTransportType urpc_get_transport_type( const char *uri );

URPC_EXPORT struct addrinfo *urpc_get_sockaddr( const char *uri );


#ifdef __cplusplus
} // extern "C"
#endif

#endif // _urpc_common_h
