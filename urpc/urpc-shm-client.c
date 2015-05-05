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

#include "urpc-shm-client.h"
#include "urpc-common.h"
#include "urpc-shm.h"
#include "urpc-semaphore.h"
#include "endianness.h"

#include <stdio.h>
#include <stdlib.h>


#if defined _MSVC_COMPILER
#define snprintf sprintf_s
#endif


#define URPC_SHM_CLIENT_TYPE 0x434D4853


typedef struct uRpcSHMTransport {

  uRpcData         *urpc_data;              // RPC данные.

  uRpcSem          *start;                  // Семафор запуска функции на выполнение.
  uRpcSem          *stop;                   // Семафор сигнализации завершения функции.
  uRpcSem          *used;                   // Признак использования.

} uRpcSHMTransport;


typedef struct uRpcSHMClient {

  uint32_t          urpc_shm_client_type;   // Тип объекта uRpcSHMClient.

  uRpcSem          *access;                 // Семафор доступа к SHM серверу.
  uRpcShm          *transport_shm;          // Сегмент разделяемой области памяти RPC данных.

  uRpcSHMTransport *transport;              // Выбранный буфер обмена с сервером.
  uRpcSHMTransport **transports;            // Сегменты обмена данными.
  uint32_t          threads_num;            // Число рабочих потоков.

} uRpcSHMClient;


uRpcSHMClient *urpc_shm_client_create( const char *uri )
{

  uRpcSHMClient *urpc_shm_client = NULL;

  uRpcShm *control_shm;

  char obj_name[ MAX_HOST_LEN + 32 ];
  uRpcSHMControl *control = NULL;
  char *transport_shm;

  uint32_t max_data_size;

  unsigned int i;

  // Проверяем тип адреса.
  if( urpc_get_type( uri ) != URPC_SHM ) return NULL;
  uri = uri + 6;

  // Структура объекта.
  urpc_shm_client = malloc( sizeof( uRpcSHMClient ) );
  if( urpc_shm_client == NULL ) return NULL;

  urpc_shm_client->urpc_shm_client_type = URPC_SHM_CLIENT_TYPE;
  urpc_shm_client->access = NULL;
  urpc_shm_client->transport_shm = NULL;
  urpc_shm_client->transport = NULL;
  urpc_shm_client->transports = NULL;

  // Считываем информацию о сервере.
  snprintf( obj_name, sizeof( obj_name ), "%s.control", uri );
  control_shm = urpc_shm_open_ro( obj_name, sizeof( uRpcSHMControl ) );
  if( control_shm == NULL ) goto urpc_shm_client_create_fail;
  control = urpc_shm_map( control_shm );
  if( control == NULL ) goto urpc_shm_client_create_fail;
  urpc_shm_client->threads_num = control->threads_num;
  max_data_size = control->size;
  urpc_shm_destroy( control_shm );

  // Открываем семафор доступа к SHM серверу.
  snprintf( obj_name, sizeof( obj_name ), "%s.access", uri );
  urpc_shm_client->access = urpc_sem_open( obj_name );
  if( urpc_shm_client->access == NULL ) goto urpc_shm_client_create_fail;

  // Подключаемся к транспортному сегменту shared memory сервера.
  // Для клиента входящий и исходящий буферы меняем местами.
  snprintf( obj_name, sizeof( obj_name ), "%s.transport", uri );
  urpc_shm_client->transport_shm = urpc_shm_open( obj_name, 2 * max_data_size * urpc_shm_client->threads_num );
  if( urpc_shm_client->transport_shm == NULL ) goto urpc_shm_client_create_fail;
  transport_shm = urpc_shm_map( urpc_shm_client->transport_shm );
  if( transport_shm == NULL ) goto urpc_shm_client_create_fail;

  urpc_shm_client->transports = malloc( urpc_shm_client->threads_num * sizeof( uRpcData* ) );
  if( urpc_shm_client->transports == NULL ) goto urpc_shm_client_create_fail;
  for( i = 0; i < urpc_shm_client->threads_num; i++ ) urpc_shm_client->transports[i] = NULL;

  for( i = 0; i < urpc_shm_client->threads_num; i++ )
    {
    urpc_shm_client->transports[i] = malloc( sizeof( uRpcSHMTransport ) );
    if( urpc_shm_client->transports[i] == NULL ) goto urpc_shm_client_create_fail;
    urpc_shm_client->transports[i]->urpc_data = NULL;
    urpc_shm_client->transports[i]->start = NULL;
    urpc_shm_client->transports[i]->stop = NULL;
    urpc_shm_client->transports[i]->used = NULL;
    }

  for( i = 0; i < urpc_shm_client->threads_num; i++ )
    {

    char *obuffer = transport_shm + i * 2 * max_data_size;
    char *ibuffer = obuffer +  max_data_size;
    urpc_shm_client->transports[i]->urpc_data = urpc_data_create( max_data_size, sizeof( uRpcHeader ), ibuffer, obuffer, 0 );
    if( urpc_shm_client->transports[i]->urpc_data == NULL ) goto urpc_shm_client_create_fail;

    snprintf( obj_name, sizeof( obj_name ), "%s.transport.%u.start", uri, i );
    urpc_shm_client->transports[i]->start = urpc_sem_open( obj_name );
    if( urpc_shm_client->transports[i]->start == NULL ) goto urpc_shm_client_create_fail;

    snprintf( obj_name, sizeof( obj_name ), "%s.transport.%u.stop", uri, i );
    urpc_shm_client->transports[i]->stop = urpc_sem_open( obj_name );
    if( urpc_shm_client->transports[i]->stop == NULL ) goto urpc_shm_client_create_fail;

    snprintf( obj_name, sizeof( obj_name ), "%s.transport.%u.used", uri, i );
    urpc_shm_client->transports[i]->used = urpc_sem_open( obj_name );
    if( urpc_shm_client->transports[i]->used == NULL ) goto urpc_shm_client_create_fail;

    }

  return urpc_shm_client;

  urpc_shm_client_create_fail:
    urpc_shm_client_destroy( urpc_shm_client );

  return NULL;

}


void urpc_shm_client_destroy( uRpcSHMClient *urpc_shm_client )
{

  unsigned int i;

  if( urpc_shm_client->urpc_shm_client_type != URPC_SHM_CLIENT_TYPE ) return;

  if( urpc_shm_client->transports != NULL )
    {
    for( i = 0; i < urpc_shm_client->threads_num; i++ )
      {
      if( urpc_shm_client->transports[i] == NULL ) continue;
      if( urpc_shm_client->transports[i]->urpc_data != NULL ) urpc_data_destroy( urpc_shm_client->transports[i]->urpc_data );
      if( urpc_shm_client->transports[i]->start != NULL ) urpc_sem_destroy( urpc_shm_client->transports[i]->start );
      if( urpc_shm_client->transports[i]->stop != NULL ) urpc_sem_destroy( urpc_shm_client->transports[i]->stop );
      if( urpc_shm_client->transports[i]->used != NULL ) urpc_sem_destroy( urpc_shm_client->transports[i]->used );
      free( urpc_shm_client->transports[i] );
      }
    free( urpc_shm_client->transports );
    }

  if( urpc_shm_client->transport_shm != NULL ) urpc_shm_destroy( urpc_shm_client->transport_shm );
  if( urpc_shm_client->access != NULL ) urpc_sem_destroy( urpc_shm_client->access );

  free( urpc_shm_client );

}


uRpcData *urpc_shm_client_lock( uRpcSHMClient *urpc_shm_client )
{

  unsigned int i;

  if( urpc_shm_client->urpc_shm_client_type != URPC_SHM_CLIENT_TYPE ) return NULL;

  // Блокируем доступ к серверу.
  urpc_sem_lock( urpc_shm_client->access );

  // Ищем свободный буфер обмена с сервером.
  for( i = 0; i < urpc_shm_client->threads_num; i++ )
    if( urpc_sem_trylock( urpc_shm_client->transports[i]->used ) == 0 )
      break;

  // Нет свободных буферов !!??
  if( i == urpc_shm_client->threads_num ) return NULL;

  urpc_shm_client->transport = urpc_shm_client->transports[i];

  return urpc_shm_client->transport->urpc_data;

}


uint32_t urpc_shm_client_exchange( uRpcSHMClient *urpc_shm_client )
{

  uRpcHeader *iheader;

  if( urpc_shm_client->urpc_shm_client_type != URPC_SHM_CLIENT_TYPE ) return URPC_STATUS_FAIL;

  iheader = urpc_data_get_header( urpc_shm_client->transport->urpc_data, URPC_DATA_INPUT );

  // Сигнализируем о начале выполнения запроса.
  urpc_sem_unlock( urpc_shm_client->transport->start );

  // Ожидаем завершения выполнения.
  urpc_sem_lock( urpc_shm_client->transport->stop );

  // Проверяем заголовок ответа.
  if( UINT32_FROM_BE( iheader->magic ) != URPC_MAGIC ) return URPC_STATUS_TRANSPORT_ERROR;

  urpc_data_set_data_size( urpc_shm_client->transport->urpc_data, URPC_DATA_INPUT, UINT32_FROM_BE( iheader->size ) - URPC_HEADER_SIZE );

  return URPC_STATUS_OK;

}


void urpc_shm_client_unlock( uRpcSHMClient *urpc_shm_client )
{

  if( urpc_shm_client->urpc_shm_client_type != URPC_SHM_CLIENT_TYPE ) return;

  urpc_sem_unlock( urpc_shm_client->transport->used );
  urpc_sem_unlock( urpc_shm_client->access );
  urpc_shm_client->transport = NULL;

}
