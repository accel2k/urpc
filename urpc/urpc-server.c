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

#include "urpc-server.h"
#include "urpc-common.h"
#include "urpc-thread.h"
#include "urpc-hash-table.h"
#include "urpc-mutex.h"
#include "urpc-timer.h"
#include "endianness.h"

#include "urpc-udp-server.h"

#include <stdio.h>
#include <stdlib.h>


#define URPC_SERVER_TYPE 0x53504455


typedef struct uRpcServerSession {

  uRpcTimer        *activity;               // Время последней активности.

} uRpcServerSession;


typedef struct uRpcServer {

  uint32_t          urpc_server_type;       // Тип объекта uRpcServer.

  char             *uri;                    // Адрес сервера.
  uRpcType          type;                   // ип протокола RPC.

  uRpcHashTable    *procs;                  // Пользовательские функции.
  uRpcHashTable    *pdata;                  // Данные для пользовательских функций.

  uint32_t          threads_num;            // Число рабочих потоков.
  uint32_t          max_data_size;          // Максимальный размер данных в RPC запросе/ответе.
  double            data_timeout;           // Таймаут ожидания передачи данных.
  void             *transport;              // Указатель на один из объектов: uRpcUDPServer, uRpcTCPServer, uRpcSHMServer.

  uRpcThread      **servers;                // Рабочие потоки.
  volatile uint32_t started_servers;        // Число запущенных потоков.
  volatile uint32_t shutdown;               // Признак завершения работы.
  uRpcMutex         lock;                   // Блокировка доступа к критическим данным структуры.

} uRpcServer;


// Функция обмена данными в потоке.
static void *urpc_server_func( void *data )
{

  uRpcServer *urpc_server = data;

  uint32_t thread_id;
  uint32_t status;

  uRpcData *urpc_data;
  uRpcHeader *iheader;
  uRpcHeader *oheader;
  uint32_t send_size;

  uint32_t session;
  uint32_t proc_id;
  urpc_server_callback proc;
  void *proc_data;

  // Сигнализация о запуске потока.
  urpc_mutex_lock( &urpc_server->lock );
  thread_id = urpc_server->started_servers++;
  urpc_mutex_unlock( &urpc_server->lock );

  while( !urpc_server->shutdown )
    {

    // Ожидание запроса от клиента.
    switch( urpc_server->type )
      {
      case URPC_UDP: urpc_data = urpc_udp_server_recv( urpc_server->transport, thread_id ); break;
      default: urpc_data = NULL; break;
      }

    // Если в течение периода ожидания запроса не поступило, проверяем флаг завершения
    // потока и возвращаемся к ожиданию запросов.
    if( urpc_data == NULL ) continue;

    iheader = urpc_data_get_header( urpc_data, URPC_DATA_INPUT );
    oheader = urpc_data_get_header( urpc_data, URPC_DATA_OUTPUT );
    session = UINT32_FROM_BE( iheader->session );

    status = URPC_STATUS_FAIL;
    urpc_data_set_uint32( urpc_data, URPC_PARAM_STATUS, status );

    // Проверяем версию клиента.
    if( ( UINT32_FROM_BE( iheader->version ) >> 8 ) != ( URPC_VERSION >> 8 ) )
      {
      status = URPC_STATUS_VERSION_MISMATCH;
      goto urpc_server_send_reply;
      }

    // Запрашиваемая функция.
    proc_id = urpc_data_get_uint32( urpc_data, URPC_PARAM_PROC );
/*
    // Запрос возможностей сервера.
    if( session == 0 && proc_id == URPC_PROC_GET_CAP )
      {
      urpc_data_set_uint32( urpc_data, URPC_PARAM_CAP, 0 );
      status = URPC_STATUS_OK;
      goto urpc_server_send_reply;
      }

    // Начало сессии.
    if( session == 0 && proc_id == URPC_PROC_LOGIN )
      {
      status = URPC_STATUS_OK;
      goto urpc_server_send_reply;
      }

    // Проверка наличия сессии.
    if( session == 0 )
      {
      status = URPC_STATUS_AUTH_ERROR;
      goto urpc_server_send_reply;
      }
*/
    #warning "Add authentication and decryption here!!!"

    // Вызов пользовательской функциии.
    proc = urpc_hash_table_find( urpc_server->procs, proc_id );
    proc_data = urpc_hash_table_find( urpc_server->pdata, proc_id );
    if( proc != NULL )
      if( proc( session, urpc_data, proc_data, NULL ) == 0 ) status = URPC_STATUS_OK;

    #warning "Add authentication and encryption here!!!"

    // Отправка ответа.
    urpc_server_send_reply:

      urpc_data_set_uint32( urpc_data, URPC_PARAM_STATUS, status );

      // Заголовок отправляемого пакета.
      send_size = URPC_HEADER_SIZE + urpc_data_get_data_size( urpc_data, URPC_DATA_OUTPUT );
      oheader->magic = UINT32_TO_BE( URPC_MAGIC );
      oheader->version = UINT32_TO_BE( URPC_VERSION );
      oheader->size = UINT32_TO_BE( send_size );
      oheader->session = iheader->session;

      // Отправка ответа.
      switch( urpc_server->type )
        {
        case URPC_UDP: urpc_udp_server_send( urpc_server->transport, thread_id ); break;
        default: break;
        }

    }

  // Сигнализация о завершении потока.
  urpc_mutex_lock( &urpc_server->lock );
  urpc_server->started_servers--;
  urpc_mutex_unlock( &urpc_server->lock );

  return NULL;

}


URPC_EXPORT uRpcServer *urpc_server_create( const char *uri, uint32_t max_data_size, double data_timeout, uint32_t threads_num, uint32_t max_clients )
{

  uRpcServer *urpc_server = NULL;
  uRpcType urpc_type = URPC_UNKNOWN;

  urpc_type = urpc_get_type( uri );
  if( urpc_type == URPC_UNKNOWN ) return NULL;

  urpc_server = malloc( sizeof( uRpcServer ) );
  if( urpc_server == NULL ) return NULL;

  urpc_server->type = urpc_type;
  urpc_server->urpc_server_type = URPC_SERVER_TYPE;
  urpc_server->uri = NULL;
  urpc_server->procs = NULL;
  urpc_server->transport = NULL;
  urpc_server->servers = NULL;
  urpc_server->threads_num = threads_num;
  urpc_server->max_data_size = max_data_size;
  urpc_server->data_timeout = data_timeout;
  urpc_server->started_servers = 0;
  urpc_server->shutdown = 0;
  urpc_mutex_init( &urpc_server->lock );

  urpc_server->uri = malloc( strlen( uri ) + 1 );
  if( urpc_server->uri == NULL ) goto urpc_server_create_fail;
  memcpy( urpc_server->uri, uri, strlen( uri ) + 1 );

  urpc_server->procs = urpc_hash_table_create();
  if( urpc_server->procs == NULL ) goto urpc_server_create_fail;

  urpc_server->pdata = urpc_hash_table_create();
  if( urpc_server->pdata == NULL ) goto urpc_server_create_fail;

  urpc_server->servers = malloc( threads_num * sizeof( uRpcThread* ) );
  if( urpc_server->servers == NULL ) goto urpc_server_create_fail;

  return urpc_server;

  urpc_server_create_fail:
    urpc_server_destroy( urpc_server );

  return NULL;

}


void urpc_server_destroy( uRpcServer *urpc_server )
{

  uint32_t started_servers;
  int i;

  if( urpc_server->urpc_server_type != URPC_SERVER_TYPE ) return;

  // Сигнализируем потокам о завершении работы.
  urpc_server->shutdown = 1;

  // Ожидаем завершения работы всех потоков.
  do {

    urpc_mutex_lock( &urpc_server->lock );
    started_servers = urpc_server->started_servers;
    urpc_mutex_unlock( &urpc_server->lock );

  } while( started_servers != 0 );

  // Удаляем объекты потоков.
  for( i = 0; i < urpc_server->threads_num; i++ )
    urpc_thread_destroy( urpc_server->servers[i] );

  // Удаляем объект обмена данными.
  if( urpc_server->transport != NULL )
    {
    switch( urpc_server->type )
      {
      case URPC_UDP: urpc_udp_server_destroy( urpc_server->transport ); break;
      default: break;
      }
    }

  // Удаляем объект.
  if( urpc_server->servers != NULL ) free( urpc_server->servers );
  if( urpc_server->pdata != NULL ) urpc_hash_table_destroy( urpc_server->pdata );
  if( urpc_server->procs != NULL ) urpc_hash_table_destroy( urpc_server->procs );
  if( urpc_server->uri != NULL ) free( urpc_server->uri );

  urpc_mutex_clear( &urpc_server->lock );

  free( urpc_server );

}


int urpc_server_add_proc( uRpcServer *urpc_server, uint32_t proc_id, urpc_server_callback proc, void *proc_data )
{

  if( urpc_server->urpc_server_type != URPC_SERVER_TYPE ) return -1;
  if( urpc_hash_table_find( urpc_server->procs, proc_id ) != NULL ) return -1;

  if( urpc_hash_table_insert( urpc_server->procs, proc_id, proc ) != 0 ) return -1;
  if( urpc_hash_table_insert( urpc_server->pdata, proc_id, proc_data ) != 0 ) return -1;

  return 0;

}


int urpc_server_bind( uRpcServer *urpc_server )
{

  uint32_t started_servers = 0;
  uint32_t fail = 0;
  int i;

  if( urpc_server->urpc_server_type != URPC_SERVER_TYPE ) return -1;

  switch( urpc_server->type )
    {
    case URPC_UDP: urpc_server->transport = urpc_udp_server_create( urpc_server->uri, urpc_server->threads_num, urpc_server->max_data_size, urpc_server->data_timeout ); break;
    default: return -1;
    }

  if( urpc_server->transport == NULL ) return -1;

  for( i = 0; i < urpc_server->threads_num; i++ )
    urpc_server->servers[i] = urpc_thread_create( urpc_server_func, urpc_server );

  do {

    urpc_mutex_lock( &urpc_server->lock );
    started_servers = urpc_server->started_servers;
    urpc_mutex_unlock( &urpc_server->lock );

  } while( started_servers != urpc_server->threads_num );

  if( fail )
    {
    urpc_server->shutdown = 1;
    return -1;
    }

  return 0;

}
