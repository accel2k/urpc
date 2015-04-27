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

#include "urpc-udp-client.h"
#include "urpc-common.h"
#include "urpc-network.h"
#include "urpc-timer.h"
#include "endianness.h"

#include <stdlib.h>


#define URPC_UDP_CLIENT_TYPE 0x43504455


typedef struct uRpcUDPClient {

  uint32_t          urpc_udp_client_type;

  SOCKET            socket;

  uRpcData         *urpc_data;
  uRpcTimer        *timer;
  double            timeout;

} uRpcUDPClient;


uRpcUDPClient *urpc_udp_client_create( const char *uri, uint32_t max_data_size, double exec_timeout )
{

  uRpcUDPClient *urpc_udp_client = NULL;
  struct addrinfo *addr = NULL;

  // Проверка ограничений.
  if( max_data_size > URPC_DEFAULT_DATA_SIZE ) return NULL;
  if( exec_timeout < URPC_MIN_TIMEOUT ) exec_timeout = URPC_MIN_TIMEOUT;
  max_data_size += URPC_HEADER_SIZE;

  // Проверяем тип адреса.
  if( urpc_get_type( uri ) != URPC_UDP ) return NULL;

  // Структура объекта.
  urpc_udp_client = malloc( sizeof( uRpcUDPClient ) );
  if( urpc_udp_client == NULL ) return NULL;

  urpc_udp_client->urpc_udp_client_type = URPC_UDP_CLIENT_TYPE;
  urpc_udp_client->socket = INVALID_SOCKET;
  urpc_udp_client->urpc_data = NULL;
  urpc_udp_client->timer = NULL;

  // Буферы приёма-передачи.
  urpc_udp_client->urpc_data = urpc_data_create( max_data_size, sizeof( uRpcHeader ), NULL, NULL, 0 );
  if( urpc_udp_client->urpc_data == NULL ) goto urpc_udp_client_create_fail;

  // Адрес сервера.
  addr = urpc_get_sockaddr( uri );
  if( addr == NULL ) goto urpc_udp_client_create_fail;

  // Рабочий сокет.
  urpc_udp_client->socket = socket( addr->ai_family, SOCK_DGRAM, addr->ai_protocol );
  if( urpc_udp_client->socket == INVALID_SOCKET ) goto urpc_udp_client_create_fail;
  if( connect( urpc_udp_client->socket, addr->ai_addr, addr->ai_addrlen ) < 0 ) goto urpc_udp_client_create_fail;
  urpc_network_set_non_block( urpc_udp_client->socket );

  // Таймер передачи.
  urpc_udp_client->timer = urpc_timer_create();
  if( urpc_udp_client->timer == NULL ) goto urpc_udp_client_create_fail;

  urpc_udp_client->timeout = URPC_DEFAULT_CLIENT_TIMEOUT;

  return urpc_udp_client;

  urpc_udp_client_create_fail:
    urpc_udp_client_destroy( urpc_udp_client );
    if( addr != NULL ) freeaddrinfo( addr );

  return NULL;

}


void urpc_udp_client_destroy( uRpcUDPClient *urpc_udp_client )
{

  if( urpc_udp_client->urpc_udp_client_type != URPC_UDP_CLIENT_TYPE ) return;

  if( urpc_udp_client->socket != INVALID_SOCKET ) closesocket( urpc_udp_client->socket );
  if( urpc_udp_client->timer != NULL ) urpc_timer_destroy( urpc_udp_client->timer );
  if( urpc_udp_client->urpc_data != NULL ) urpc_data_destroy( urpc_udp_client->urpc_data );

  free( urpc_udp_client );

}


uRpcData *urpc_udp_client_lock( uRpcUDPClient *urpc_udp_client )
{

  if( urpc_udp_client->urpc_udp_client_type != URPC_UDP_CLIENT_TYPE ) return NULL;

  return urpc_udp_client->urpc_data;

}


uint32_t urpc_udp_client_exchange( uRpcUDPClient *urpc_udp_client )
{

  fd_set sock_set;
  struct timeval sock_tv;

  uRpcHeader *iheader = urpc_data_get_header( urpc_udp_client->urpc_data, URPC_DATA_INPUT );
  uRpcHeader *oheader = urpc_data_get_header( urpc_udp_client->urpc_data, URPC_DATA_OUTPUT );

  int send_size = UINT32_FROM_BE( oheader->size );
  int recv_size = 0;

  // Время начала передачи.
  urpc_timer_start( urpc_udp_client->timer );

  // Отправка запроса.
  if( send( urpc_udp_client->socket, (void*)oheader, send_size, 0 ) < 0 ) return URPC_STATUS_TRANSPORT_ERROR;

  // Ожидание ответа в течение времени urpc_udp_client->timeout.
  while( urpc_timer_elapsed( urpc_udp_client->timer ) <  urpc_udp_client->timeout )
    {

    // Проверяем приход ответа с интервалом в 100мс.
    FD_ZERO( &sock_set );
    FD_SET( urpc_udp_client->socket, &sock_set );
    sock_tv.tv_sec = 0;
    sock_tv.tv_usec = 100000;

    if( select( urpc_udp_client->socket + 1, &sock_set, NULL, NULL, &sock_tv ) < 0 )
      {
      if( urpc_network_last_error() == EINTR ) continue;
      else return URPC_STATUS_TRANSPORT_ERROR;
      }

    // Если данных нет - ждём.
    if( !FD_ISSET( urpc_udp_client->socket, &sock_set ) ) continue;

    // Считываем ответ.
    recv_size = recv( urpc_udp_client->socket, (void*)iheader, URPC_DEFAULT_BUFFER_SIZE, 0 );
    if( recv_size < 0 )
      {
      if( urpc_network_last_error() == EINTR ) continue;
      else return URPC_STATUS_TRANSPORT_ERROR;
      }

    // Проверяем заголовок ответа.
    if( UINT32_FROM_BE( iheader->size ) != recv_size ) continue;
    if( UINT32_FROM_BE( iheader->magic ) != URPC_MAGIC ) continue;

    urpc_data_set_data_size( urpc_udp_client->urpc_data, URPC_DATA_INPUT, recv_size - URPC_HEADER_SIZE );

    return URPC_STATUS_OK;

    }

  return URPC_STATUS_TIMEOUT;

}


void urpc_udp_client_unlock( uRpcUDPClient *urpc_udp_client )
{

  // Очищаем буферы приёма-передачи.
  urpc_data_set_data_size( urpc_udp_client->urpc_data, URPC_DATA_INPUT, 0 );
  urpc_data_set_data_size( urpc_udp_client->urpc_data, URPC_DATA_OUTPUT, 0 );

}
