/*
 * uRpc - rpc (remote procedure call) library.
 *
 * Copyright 2009, 2010, 2011, 2015 Andrei Fadeev
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

#ifndef _urpc_network_h
#define _urpc_network_h

#include <urpc-exports.h>


#if defined( _WIN32 )

#include <winsock2.h>
#include <ws2tcpip.h>

#define MSG_NOSIGNAL     0
#define EAGAIN           WSAEWOULDBLOCK
#define EINTR            WSAEINTR

#endif


#if defined( __unix__ )

#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>

#define SOCKET           int
#define INVALID_SOCKET   -1
#define closesocket      close
#define ioctlsocket      ioctl

#endif


#ifdef __cplusplus
extern "C" {
#endif


URPC_EXPORT int urpc_network_init( void );
URPC_EXPORT void urpc_network_close( void );

URPC_EXPORT int urpc_network_last_error( void );
URPC_EXPORT const char* urpc_network_last_error_str( void );

URPC_EXPORT int urpc_network_set_tcp_nodelay( SOCKET socket );
URPC_EXPORT int urpc_network_set_reuse( SOCKET socket );
URPC_EXPORT int urpc_network_set_non_block( SOCKET socket );


#ifdef __cplusplus
} // extern "C"
#endif

#endif // _urpc_network_h
