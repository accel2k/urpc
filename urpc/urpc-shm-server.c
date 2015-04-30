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

#include "urpc-shm-server.h"
#include "urpc-common.h"
#include "urpc-shm.h"
#include "urpc-semaphore.h"
#include "endianness.h"

#include <stdio.h>
#include <stdlib.h>

#if defined( __unix__ )
#include <signal.h>
#endif


#if defined _MSVC_COMPILER
#define snprintf sprintf_s
#endif


#define URPC_SHM_SERVER_TYPE 0x534D4853


typedef struct uRpcSHMTransport {

  uRpcData         *urpc_data;              // RPC данные.

  uRpcSem          *start;                  // Семафор запуска функции на выполнение.
  uRpcSem          *stop;                   // Семафор сигнализации завершения функции.
  uRpcSem          *used;                   // Признак использования.

} uRpcSHMTransport;


typedef struct uRpcSHMServer {

  uint32_t          urpc_shm_server_type;   // Тип объекта uRpcSHMServer.

  uRpcSem          *access;                 // Семафор доступа к SHM серверу.

  uRpcShm          *control;                // Сегмент разделяемой области памяти управляющей структуры.
  uRpcShm          *transport;              // Сегмент разделяемой области памяти RPC данных.

  uRpcSHMTransport **transports;            // Сегменты обмена данными.
  uint32_t          threads_num;            // Число рабочих потоков.

} uRpcSHMServer;


uRpcSHMServer *urpc_shm_server_create( const char *uri, uint32_t threads_num, uint32_t max_data_size )
{

  uRpcSHMServer *urpc_shm_server = NULL;

  char obj_name[ MAX_HOST_LEN + 32 ];
  uRpcSHMControl *control = NULL;
  char *transport_shm;

  unsigned int i;

  // Проверка ограничений.
  if( max_data_size > URPC_MAX_DATA_SIZE ) return NULL;
  if( threads_num > URPC_MAX_THREADS_NUM ) threads_num = URPC_MAX_THREADS_NUM;
  max_data_size += URPC_HEADER_SIZE;

  // Проверяем тип адреса.
  if( urpc_get_type( uri ) != URPC_SHM ) return NULL;
  uri = uri + 6;

  // Структура объекта.
  urpc_shm_server = malloc( sizeof( uRpcSHMServer ) );
  if( urpc_shm_server == NULL ) return NULL;

  urpc_shm_server->urpc_shm_server_type = URPC_SHM_SERVER_TYPE;
  urpc_shm_server->access = NULL;
  urpc_shm_server->control = NULL;
  urpc_shm_server->transport = NULL;
  urpc_shm_server->transports = NULL;
  urpc_shm_server->threads_num = threads_num;

  // Название управляющего сегмента.
  snprintf( obj_name, sizeof( obj_name ), "%s.control", uri );

  // Проверяем, что нет другой копии сервера по этому адресу. В Linux сегмент
  // разделяемой памяти может остаться после аварийного завершения работы.
  #if defined( __unix__ )
  uRpcShm *control_shm = urpc_shm_open_ro( obj_name, sizeof( uRpcSHMControl ) );
  if( control_shm != NULL )
    {
    control = urpc_shm_map( control_shm );
    if( control == NULL ) goto urpc_shm_server_create_fail;
    if( kill( control->pid, 0 ) == 0 ) goto urpc_shm_server_create_fail;
    urpc_shm_destroy( control_shm );
    }
  #endif

  // Создаём управляющий сегмент SHM сервера.
  urpc_shm_remove( obj_name );
  urpc_shm_server->control = urpc_shm_create( obj_name, sizeof( uRpcSHMControl ) );
  if( urpc_shm_server->control == NULL ) goto urpc_shm_server_create_fail;
  control = urpc_shm_map( urpc_shm_server->control );
  if( control == NULL ) goto urpc_shm_server_create_fail;

  // Создаем семафор доступа к SHM серверу.
  snprintf( obj_name, sizeof( obj_name ), "%s.access", uri );
  urpc_sem_remove( obj_name );
  urpc_shm_server->access = urpc_sem_create( obj_name, URPC_SEM_UNLOCKED, threads_num );
  if( urpc_shm_server->access == NULL ) goto urpc_shm_server_create_fail;

  // Параметры сервера.
  #if defined( __unix__ )
  control->pid = getpid();
  #else
  control->pid = 0;
  #endif
  control->size = max_data_size;
  control->threads_num = threads_num;

  // Создаем транспортный сегмент SHM сервера.
  // Сегмент содержит по два буфера размером max_data_size для каждого потока.
  snprintf( obj_name, sizeof( obj_name ), "%s.transport", uri );
  urpc_shm_remove( obj_name );
  urpc_shm_server->transport = urpc_shm_create( obj_name, 2 * max_data_size * threads_num );
  if( urpc_shm_server->transport == NULL ) goto urpc_shm_server_create_fail;
  transport_shm = urpc_shm_map( urpc_shm_server->transport );
  if( transport_shm == NULL ) goto urpc_shm_server_create_fail;

  // Буферы приёма-передачи, семафоры вызова функций.
  urpc_shm_server->transports = malloc( threads_num * sizeof( uRpcData* ) );
  if( urpc_shm_server->transports == NULL ) goto urpc_shm_server_create_fail;
  for( i = 0; i < threads_num; i++ ) urpc_shm_server->transports[i] = NULL;

  for( i = 0; i < threads_num; i++ )
    {
    urpc_shm_server->transports[i] = malloc( sizeof( uRpcSHMTransport ) );
    if( urpc_shm_server->transports[i] == NULL ) goto urpc_shm_server_create_fail;
    urpc_shm_server->transports[i]->urpc_data = NULL;
    urpc_shm_server->transports[i]->start = NULL;
    urpc_shm_server->transports[i]->stop = NULL;
    urpc_shm_server->transports[i]->used = NULL;
    }

  for( i = 0; i < threads_num; i++ )
    {

    char *ibuffer = transport_shm + i * 2 * max_data_size;
    char *obuffer = ibuffer +  max_data_size;
    urpc_shm_server->transports[i]->urpc_data = urpc_data_create( max_data_size, sizeof( uRpcHeader ), ibuffer, obuffer, 0 );
    if( urpc_shm_server->transports[i]->urpc_data == NULL ) goto urpc_shm_server_create_fail;

    snprintf( obj_name, sizeof( obj_name ), "%s.transport.%u.start", uri, i );
    urpc_sem_remove( obj_name );
    urpc_shm_server->transports[i]->start = urpc_sem_create( obj_name, URPC_SEM_LOCKED, 1 );
    if( urpc_shm_server->transports[i]->start == NULL ) goto urpc_shm_server_create_fail;

    snprintf( obj_name, sizeof( obj_name ), "%s.transport.%u.stop", uri, i );
    urpc_sem_remove( obj_name );
    urpc_shm_server->transports[i]->stop = urpc_sem_create( obj_name, URPC_SEM_LOCKED, 1 );
    if( urpc_shm_server->transports[i]->stop == NULL ) goto urpc_shm_server_create_fail;

    snprintf( obj_name, sizeof( obj_name ), "%s.transport.%u.used", uri, i );
    urpc_sem_remove( obj_name );
    urpc_shm_server->transports[i]->used = urpc_sem_create( obj_name, URPC_SEM_UNLOCKED, 1 );
    if( urpc_shm_server->transports[i]->used == NULL ) goto urpc_shm_server_create_fail;

    }

  return urpc_shm_server;

  urpc_shm_server_create_fail:
    urpc_shm_server_destroy( urpc_shm_server );

  return NULL;

}


void urpc_shm_server_destroy( uRpcSHMServer *urpc_shm_server )
{

  unsigned int i;

  if( urpc_shm_server->urpc_shm_server_type != URPC_SHM_SERVER_TYPE ) return;

  if( urpc_shm_server->transports != NULL )
    {
    for( i = 0; i < urpc_shm_server->threads_num; i++ )
      {
      if( urpc_shm_server->transports[i] == NULL ) continue;
      if( urpc_shm_server->transports[i]->urpc_data != NULL ) urpc_data_destroy( urpc_shm_server->transports[i]->urpc_data );
      if( urpc_shm_server->transports[i]->start != NULL ) urpc_sem_destroy( urpc_shm_server->transports[i]->start );
      if( urpc_shm_server->transports[i]->stop != NULL ) urpc_sem_destroy( urpc_shm_server->transports[i]->stop );
      if( urpc_shm_server->transports[i]->used != NULL ) urpc_sem_destroy( urpc_shm_server->transports[i]->used );
      free( urpc_shm_server->transports[i] );
      }
    free( urpc_shm_server->transports );
    }

  if( urpc_shm_server->transport != NULL ) urpc_shm_destroy( urpc_shm_server->transport );
  if( urpc_shm_server->access != NULL ) urpc_sem_destroy( urpc_shm_server->access );
  if( urpc_shm_server->control != NULL ) urpc_shm_destroy( urpc_shm_server->control );

  free( urpc_shm_server );

}


uRpcData *urpc_shm_server_recv( uRpcSHMServer *urpc_shm_server, uint32_t thread_id )
{

  uRpcHeader *iheader;

  if( urpc_shm_server->urpc_shm_server_type != URPC_SHM_SERVER_TYPE ) return NULL;
  if( thread_id > urpc_shm_server->threads_num - 1 ) return NULL;

  printf( "thread id %d check start\n", thread_id );

  // Ждём 500мс сигнала о начале выполнения запроса.
  if( urpc_sem_timedlock( urpc_shm_server->transports[thread_id]->start, 0.5 ) != 0 ) return NULL;

  // Проверяем заголовок запроса.
  iheader = urpc_data_get_header( urpc_shm_server->transports[thread_id]->urpc_data, URPC_DATA_INPUT );
  if( UINT32_FROM_BE( iheader->magic ) != URPC_MAGIC ) return NULL;

  urpc_data_set_data_size( urpc_shm_server->transports[thread_id]->urpc_data, URPC_DATA_INPUT, UINT32_FROM_BE( iheader->size ) - URPC_HEADER_SIZE );

  return urpc_shm_server->transports[thread_id]->urpc_data;

}


int urpc_shm_server_send( uRpcSHMServer *urpc_shm_server, uint32_t thread_id )
{

  if( urpc_shm_server->urpc_shm_server_type != URPC_SHM_SERVER_TYPE ) return -1;
  if( thread_id > urpc_shm_server->threads_num - 1 ) return -1;

  // Сигналазируем о завершении выполнения запроса.
  urpc_sem_unlock( urpc_shm_server->transports[thread_id]->stop );

  return 0;

}
