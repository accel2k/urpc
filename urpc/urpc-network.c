/*
 * uRPC - rpc (remote procedure call) library.
 *
 * Copyright 2009-2015 Andrei Fadeev (andrei@webcontrol.ru)
 *
 * This file is part of uRPC.
 *
 * uRPC is dual-licensed: you can redistribute it and/or modify
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
 * Alternatively, you can license this code under a commercial license.
 * Contact the author in this case.
 *
*/

#include "urpc-network.h"


int urpc_network_set_tcp_nodelay( SOCKET socket )
{

  int flag = 1;
  return setsockopt( socket, IPPROTO_TCP, TCP_NODELAY, (void *)&flag, sizeof( flag ) );

}


int urpc_network_set_reuse( SOCKET socket )
{

  int flag = 1;
  return setsockopt( socket, SOL_SOCKET, SO_REUSEADDR, (void *)&flag, sizeof( flag ) );

}


int urpc_network_set_non_block( SOCKET socket )
{

  int flag = 1;
  return ioctlsocket( socket, FIONBIO, (void*)&flag );

}
