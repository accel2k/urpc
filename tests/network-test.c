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

#include <stdio.h>
#include <stdlib.h>
#include "urpc-network.h"
#include "urpc-thread.h"
#include "urpc-common.h"


volatile int start = 0;


void* server_thread( void *data )
{

  char *uri = data;

  SOCKET client;
  SOCKET listener;
  struct addrinfo *addr;
  uRpcType urpc_type = urpc_get_type( uri );
  int proto_style;

  struct sockaddr *client_addr;
  socklen_t client_addr_len;

  char buffer[128];

  if( urpc_type != URPC_UDP && urpc_type != URPC_TCP ) return NULL;
  if( urpc_type == URPC_UDP ) proto_style = SOCK_DGRAM;
  if( urpc_type == URPC_TCP ) proto_style = SOCK_STREAM;

  if( ( addr = urpc_get_sockaddr( uri ) ) == NULL )
    {
    printf( "can't get address info for %s\n", uri );
    return NULL;
    }

  if( ( listener = socket( addr->ai_family, proto_style, addr->ai_protocol ) ) == INVALID_SOCKET )
    {
    printf( "can't create listener socket\n" );
    return NULL;
    }

  if( urpc_network_set_reuse( listener ) != 0 )
    {
    printf( "can't reuse address at %s\n", uri );
    return NULL;
    }

  if( bind( listener, addr->ai_addr, addr->ai_addrlen ) != 0 )
    {
    printf( "can't bind listener socket %d\n", errno );
    return NULL;
    }

  client_addr = malloc( addr->ai_addrlen );
  client_addr_len = addr->ai_addrlen;
  freeaddrinfo( addr );

  if( urpc_type == URPC_TCP )
    if( listen( listener, 1 ) != 0 )
      {
      printf( "can't listen on socket\n" );
      return NULL;
      }

  start = 1;

  if( urpc_type == URPC_TCP )
    while( ( client = accept( listener, client_addr, &client_addr_len ) ) < 0  );
  else
    client = listener;

  if( urpc_type == URPC_TCP )
    if( send( client, "server say hello", 17, 0 ) < 0 )
      {
      printf( "server failed to send data\n" );
      return NULL;
      }

  memset( buffer, 0, sizeof( buffer ) );
  if( recvfrom( client, buffer, sizeof( buffer ), 0, client_addr, &client_addr_len ) < 0)
    {
    printf( "server failed to receive data %d\n", errno );
    return NULL;
    }
  else
    printf( "client data: %s\n", buffer );

  if( urpc_type == URPC_UDP )
    if( sendto( client, "server say hello", 17, 0, client_addr, client_addr_len ) < 0 )
      {
      printf( "server failed to send data %d\n", errno );
      return NULL;
      }

  closesocket( listener );
  closesocket( client );

  return NULL;

}


void* client_thread( void *data )
{

  char *uri = data;

  SOCKET client;
  struct addrinfo *addr;
  uRpcType urpc_type = urpc_get_type( uri );
  int proto_style;

  char buffer[128];

  if( urpc_type != URPC_UDP && urpc_type != URPC_TCP ) return NULL;
  if( urpc_type == URPC_UDP ) proto_style = SOCK_DGRAM;
  if( urpc_type == URPC_TCP ) proto_style = SOCK_STREAM;

  if( ( addr = urpc_get_sockaddr( uri ) ) == NULL )
    {
    printf( "can't get address info for %s\n", uri );
    return NULL;
    }

  if( ( client = socket( addr->ai_family, proto_style, addr->ai_protocol ) ) == INVALID_SOCKET )
    {
    printf( "can't create client socket\n" );
    return NULL;
    }

  while( start == 0 );

  if( connect( client, addr->ai_addr, addr->ai_addrlen ) < 0 )
    {
    printf( "can't connect to server socket\n" );
    return NULL;
    }

  if( send( client, "client say hello", 17, 0 ) < 0 )
    {
    printf( "client failed to send data \n" );
    return NULL;
    }

  memset( buffer, 0, sizeof( buffer ) );
  if( recv( client, buffer, sizeof( buffer ), 0 ) < 0)
    {
    printf( "client failed to receive data\n" );
    return NULL;
    }
  else
    printf( "server data: %s\n", buffer );

  closesocket( client );

  return NULL;

}


int main( int argc, char **argv )
{

  uRpcThread *server;
  uRpcThread *client;

  if( argc != 2 )
    {
    printf( "usage: %s <uri>\n", argv[0] );
    return 0;
    }

  urpc_network_init();

  server = urpc_thread_create( server_thread, argv[1] );
  client = urpc_thread_create( client_thread, argv[1] );

  urpc_thread_destroy( server );
  urpc_thread_destroy( client );

  urpc_network_close();

  return 0;

}
