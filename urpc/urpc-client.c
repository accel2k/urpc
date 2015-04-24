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

#include "urpc-client.h"
#include "urpc-common.h"
#include "urpc-mutex.h"
#include "endianness.h"

#include "urpc-udp-client.h"

#include <stdlib.h>


#define URPC_CLIENT_TYPE 0x4E4C4355


typedef struct uRpcClient {

  uint32_t          urpc_client_type;       // Тип объекта uRpcClient.

  char             *uri;                    // Адрес сервера.
  uRpcType          type;                   // Тип протокола RPC.

  uint32_t          max_data_size;          // Максимальный размер данных в RPC запросе/ответе.
  double            exec_timeout;           // Таймаут ожидания ответа сервера.
  void             *transport;              // Указатель на один из объектов: uRpcUDPClient, uRpcTCPClient, uRpcSHMClient.

  uRpcMutex         lock;                   // Блокировка канала передачи.
  uRpcData         *urpc_data;              // Данные RPC запроса/ответа.

  uint32_t          session;                // Идентификатор сессии.

} uRpcClient;


uRpcClient *urpc_client_create( const char *uri, uint32_t max_data_size, double exec_timeout )
{

  uRpcClient *urpc_client = NULL;
  uRpcType urpc_type = URPC_UNKNOWN;

  // Проверка типа адреса.
  urpc_type = urpc_get_type( uri );
  if( urpc_type == URPC_UNKNOWN ) return NULL;

  // Структура объекта.
  urpc_client = malloc( sizeof( uRpcClient ) );
  if( urpc_client == NULL ) return NULL;

  urpc_client->type = urpc_type;
  urpc_client->urpc_client_type = URPC_CLIENT_TYPE;
  urpc_client->uri = NULL;
  urpc_client->max_data_size = max_data_size;
  urpc_client->exec_timeout = exec_timeout;
  urpc_client->transport = NULL;
  urpc_client->urpc_data = NULL;
  urpc_client->session = 0;
  urpc_mutex_init( &urpc_client->lock );

  urpc_client->uri = malloc( strlen( uri ) + 1 );
  if( urpc_client->uri == NULL ) goto urpc_client_create_fail;
  memcpy( urpc_client->uri, uri, strlen( uri ) + 1 );

  return urpc_client;

  urpc_client_create_fail:
    urpc_client_destroy( urpc_client );

  return NULL;

}


void urpc_client_destroy( uRpcClient *urpc_client )
{

  if( urpc_client->urpc_client_type != URPC_CLIENT_TYPE ) return;

  #warning "Add logout call here!!!"

  if( urpc_client->transport != NULL )
    {
    switch( urpc_client->type )
      {
      case URPC_UDP: urpc_udp_client_destroy( urpc_client->transport ); break;
      default: break;
      }
    }

  if( urpc_client->uri != NULL ) free( urpc_client->uri );

  urpc_mutex_clear( &urpc_client->lock );

  free( urpc_client );

}


int urpc_client_connect( uRpcClient *urpc_client )
{

  if( urpc_client->urpc_client_type != URPC_CLIENT_TYPE ) return -1;

  switch( urpc_client->type )
    {
    case URPC_UDP: urpc_client->transport = urpc_udp_client_create( urpc_client->uri, urpc_client->max_data_size, urpc_client->exec_timeout ); break;
    default: return -1;
    }

  if( urpc_client->transport == NULL ) return -1;

  return 0;

}


uRpcData *urpc_client_lock( uRpcClient *urpc_client )
{

  if( urpc_client->urpc_client_type != URPC_CLIENT_TYPE ) return NULL;

  urpc_mutex_lock( &urpc_client->lock );
  urpc_client->urpc_data = NULL;

  switch( urpc_client->type )
    {
    case URPC_UDP: urpc_client->urpc_data = urpc_udp_client_lock( urpc_client->transport ); break;
    default: break;
    }

  if( urpc_client->urpc_data == NULL ) urpc_mutex_unlock( &urpc_client->lock );
  else urpc_data_set_uint32( urpc_client->urpc_data, URPC_PARAM_PROC, 0 );

  return urpc_client->urpc_data;

}


uint32_t urpc_client_exec( uRpcClient *urpc_client, uint32_t proc_id )
{

  uRpcHeader *iheader;
  uRpcHeader *oheader;
  uint32_t send_size;
  uint32_t status;

  if( urpc_client->urpc_client_type != URPC_CLIENT_TYPE ) return URPC_STATUS_FAIL;

  iheader = urpc_data_get_header( urpc_client->urpc_data, URPC_DATA_INPUT );
  oheader = urpc_data_get_header( urpc_client->urpc_data, URPC_DATA_OUTPUT );
  send_size = URPC_HEADER_SIZE + urpc_data_get_data_size( urpc_client->urpc_data, URPC_DATA_OUTPUT );

  // Заголовок отправляемого пакета.
  oheader->magic = UINT32_TO_BE( URPC_MAGIC );
  oheader->version = UINT32_TO_BE( URPC_VERSION );
  oheader->size = UINT32_TO_BE( send_size );
  oheader->session = urpc_client->session;

  urpc_data_set_uint32( urpc_client->urpc_data, URPC_PARAM_PROC, proc_id );

  #warning "Add authentication and encryption here!!!"

  // Обмен данными с сервером. Перед обменом должен быть заполнен заголовок отправляемых данных!!!
  switch( urpc_client->type )
    {
    case URPC_UDP: status = urpc_udp_client_exchange( urpc_client->transport ); break;
    default: return URPC_STATUS_FAIL;
    }
  if( status != URPC_STATUS_OK ) return status;

  // Проверка принятых данных.
  if( ( UINT32_FROM_BE( iheader->version ) >> 8 ) != ( URPC_VERSION >> 8 ) ) return URPC_STATUS_VERSION_MISMATCH;
  if( UINT32_FROM_BE( iheader->session ) != urpc_client->session ) return URPC_STATUS_AUTH_ERROR;

  #warning "Add authentication and decryption here!!!"

  if( urpc_data_validate( urpc_client->urpc_data, URPC_DATA_INPUT ) < 0 ) return URPC_STATUS_TRANSPORT_ERROR;

  return URPC_STATUS_OK;

}


void urpc_client_unlock( uRpcClient *urpc_client )
{

  if( urpc_client->urpc_client_type != URPC_CLIENT_TYPE ) return;
  if( urpc_client->urpc_data == NULL ) return;

  switch( urpc_client->type )
    {
    case URPC_UDP: urpc_udp_client_unlock( urpc_client->transport ); break;
    default: break;
    }

  urpc_client->urpc_data = NULL;
  urpc_mutex_unlock( &urpc_client->lock );

}