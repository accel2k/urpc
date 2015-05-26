/*
 * uRPC - rpc (remote procedure call) library.
 *
 * Copyright 2015 Andrei Fadeev (andrei@webcontrol.ru)
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

#include <stdio.h>
#include "urpc-common.h"


int main( int argc, char **argv )
{

  struct addrinfo *addr;
  char *uri;

  urpc_network_init();

  uri = "tcp://[::1234:7890]:12345/";
  addr = urpc_get_sockaddr( uri );
  freeaddrinfo( addr );

  uri = "tcp://127.0.0.1:12345/";
  addr = urpc_get_sockaddr( uri );
  freeaddrinfo( addr );

  uri = "tcp://localhost:12345/";
  addr = urpc_get_sockaddr( uri );
  freeaddrinfo( addr );

  uri = "tcp://www.ya.ru:12345/";
  addr = urpc_get_sockaddr( uri );
  freeaddrinfo( addr );

  uri = "tcp://*:12345/";
  addr = urpc_get_sockaddr( uri );
  freeaddrinfo( addr );

  urpc_network_close();

  return 0;

}
