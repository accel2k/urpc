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

#include <stdio.h>
#include "urpc-server.h"
#include "urpc-common.h"
#include "urpc-thread.h"
#include "urpc-mutex.h"
#include "urpc-rwmutex.h"
#include "urpc-timer.h"
#include "urpc-hash-table.h"
#include "urpc-mem-chunk.h"
#include "urpc-network.h"
#include "urpc-endian.h"

#include "urpc-udp-server.h"
#include "urpc-tcp-server.h"
#include "urpc-shm-server.h"

#include <stdlib.h>

#define URPC_SERVER_TYPE 0x53504455

static int urpc_server_initialized = 0;

typedef struct uRpcServerSession
{
  uint32_t             state;                  /* Состояние подключения. */
  uRpcTimer           *activity;               /* Время последней активности. */
  SOCKET               socket;                 /* Для TCP/IP соединения сокет подключения клиекнта. */

  void                *user_data;              /* Пользовательскте данные сессии. */

  uRpcMemChunk        *sessions_chunks;        /* Аллокатор данных сессий. */
} uRpcServerSession;

struct _uRpcServer
{
  uint32_t             urpc_server_type;       /* Тип объекта uRpcServer. */

  char                *uri;                    /* Адрес сервера. */
  uRpcType             type;                   /* Тип протокола RPC. */

  urpc_thread_proc     thread_start_proc;      /* Функция вызываемая при запуске рабочего потока. */
  void                *thread_start_proc_data; /* Пользовательские данные. */

  urpc_free_proc       thread_stop_proc;       /* Функция вызываемая при остановке рабочего потока. */
  void                *thread_stop_proc_data;  /* Пользовательские данные. */

  urpc_connect_proc    connect_proc;           /* Функция вызываемая при подключении клиента. */
  void                *connect_proc_data;      /* Пользовательские данные. */

  urpc_free_proc       disconnect_proc;        /* Функция вызываемая при отключении клиента. */
  void                *disconnect_proc_data;   /* Пользовательские данные. */

  uRpcHashTable       *procs;                  /* Пользовательские функции. */
  uRpcHashTable       *procs_data;             /* Данные для пользовательских функций. */

  uRpcHashTable       *sessions;               /* Пользовательские сессии. */
  uRpcMemChunk        *sessions_chunks;        /* Аллокатор данных сессий. */
  uint32_t             last_session_id;        /* Идентификатор последней созданной сессии. */
  double               session_timeout;        /* Таймаут сессии. */
  uRpcRWMutex          sessions_lock;          /* Блокировка доступа к пользовательским сессиям. */
  uRpcThread          *session_check;          /* Поток проверки пользовательских сессий. */

  uint32_t             threads_num;            /* Число рабочих потоков. */
  uint32_t             max_clients;            /* Максимальное число подключенных клиентов. */
  uint32_t             max_data_size;          /* Максимальный размер данных в RPC запросе/ответе. */
  double               data_timeout;           /* Таймаут обмена данными. */
  void                *transport;              /* Указатель на один из объектов: uRpcUDPServer,
                                                  uRpcTCPServer, uRpcSHMServer. */

  uRpcThread         **servers;                /* Рабочие потоки. */
  volatile uint32_t    started_servers;        /* Число запущенных потоков. */
  volatile uint32_t    shutdown;               /* Признак завершения работы. */
  uRpcMutex            lock;                   /* Блокировка доступа к критическим данным структуры. */
};

/* Функция удаления данных сессии. */
static void
urpc_server_session_remove_func (uRpcServerSession *session)
{
  if (session->activity != NULL)
    urpc_timer_destroy (session->activity);
  urpc_mem_chunk_free (session->sessions_chunks, session);
}

/* Функция проверки и отключения сессии. */
static void
urpc_server_check_session (uint32_t           session_id,
                           uRpcServerSession *session,
                           uRpcServer        *urpc_server)
{
  if (urpc_timer_elapsed (session->activity) > urpc_server->session_timeout)
    {
      if (urpc_server->disconnect_proc != NULL)
        urpc_server->disconnect_proc (session->user_data, urpc_server->disconnect_proc_data);
      urpc_hash_table_remove (urpc_server->sessions, session_id);
      if (urpc_server->type == URPC_TCP)
        {
          urpc_tcp_server_remove_client (urpc_server->transport, session->socket);
          closesocket (session->socket);
        }
    }
}

/* Функция отключения клиентов по таймауту при неактивности. */
static void *
urpc_server_session_timeout_check (void *data)
{
  uRpcServer *urpc_server = data;

  int step = 0;

  /* Сигнализация о запуске потока. */
  urpc_mutex_lock (&urpc_server->lock);
  urpc_server->started_servers++;
  urpc_mutex_unlock (&urpc_server->lock);

  /* Раз в три секунды проверяются сессии. */
  while (!urpc_server->shutdown)
    {
      urpc_timer_sleep (0.1);
      step += 1;

      if (step < 30)
        continue;
      step = 0;

      urpc_rwmutex_writer_lock (&urpc_server->sessions_lock);
      urpc_hash_table_foreach (urpc_server->sessions,
                               (urpc_hash_table_foreach_callback) urpc_server_check_session,
                               urpc_server);
      urpc_rwmutex_writer_unlock (&urpc_server->sessions_lock);
    }

  // Сигнализация о завершении потока.
  urpc_mutex_lock (&urpc_server->lock);
  urpc_server->started_servers--;
  urpc_mutex_unlock (&urpc_server->lock);

  return NULL;
}

/* Функция обмена данными в потоке. */
static void *
urpc_server_func (void *data)
{
  uRpcServer *urpc_server = data;

  uint32_t thread_id;
  void *thread_data;
  uint32_t status;
  uint32_t disconnect;

  uRpcData *urpc_data;
  uRpcHeader *iheader;
  uRpcHeader *oheader;
  uint32_t send_size;

  uint32_t session_id;
  uRpcServerSession *session;
  SOCKET csocket;

  uint32_t proc_id;
  urpc_proc proc;
  void *proc_data;

  /* Пользовательская функция запуска рабочего потока. */
  if (urpc_server->thread_start_proc != NULL)
    thread_data = urpc_server->thread_start_proc (urpc_server->thread_start_proc_data);
  else
    thread_data = NULL;

  /* Сигнализация о запуске потока. */
  urpc_mutex_lock (&urpc_server->lock);
  thread_id = urpc_server->started_servers++;
  urpc_mutex_unlock (&urpc_server->lock);

  while (!urpc_server->shutdown)
    {
      status = URPC_STATUS_FAIL;
      disconnect = URPC_FALSE;
      csocket = INVALID_SOCKET;
      session = NULL;
      proc_id = 0;

      /* Ожидание запроса от клиента. */
      switch (urpc_server->type)
        {
        case URPC_UDP:
          urpc_data = urpc_udp_server_recv (urpc_server->transport, thread_id);
          break;

        case URPC_TCP:
          urpc_data = urpc_tcp_server_recv (urpc_server->transport, thread_id);
          break;

        case URPC_SHM:
          urpc_data = urpc_shm_server_recv (urpc_server->transport, thread_id);
          break;

        default:
          urpc_data = NULL;
          break;
        }

      /* Если в течение периода ожидания запроса не поступило, проверяем флаг завершения
         потока и возвращаемся к ожиданию запросов. */
      if (urpc_data == NULL)
        continue;

      /* Сокет клиента для TCP/IP. */
      if (urpc_server->type == URPC_TCP)
        csocket = urpc_tcp_server_get_client_socket (urpc_server->transport, thread_id);

      iheader = urpc_data_get_header (urpc_data, URPC_DATA_INPUT);
      oheader = urpc_data_get_header (urpc_data, URPC_DATA_OUTPUT);
      session_id = UINT32_FROM_BE (iheader->session);

      /* Проверяем версию клиента. */
      if ((UINT32_FROM_BE (iheader->version) >> 8) != (URPC_VERSION >> 8))
        {
          status = URPC_STATUS_VERSION_MISMATCH;
          goto urpc_server_send_reply;
        }

      /* Запрашиваемая функция. */
      urpc_data_get_uint32 (urpc_data, URPC_PARAM_PROC, &proc_id);

      /* Запрос возможностей сервера. */
      if (session_id == 0 && proc_id == URPC_PROC_GET_CAP)
        {
          urpc_data_set_uint32 (urpc_data, URPC_PARAM_CAP, 0);
          status = URPC_STATUS_OK;
          goto urpc_server_send_reply;
        }

      /* Начало сессии. */
      if (session_id == 0 && proc_id == URPC_PROC_LOGIN)
        {
          urpc_rwmutex_writer_lock (&urpc_server->sessions_lock);

          /* Проверка числа уже подключенных клиентов. */
          if (urpc_hash_table_size (urpc_server->sessions) >= urpc_server->max_clients)
            {
              urpc_rwmutex_writer_unlock (&urpc_server->sessions_lock);
              status = URPC_STATUS_TOO_MANY_CONNECTIONS;
              goto urpc_server_send_reply;
            }

          /* Структура с новой сессией. */
          session = urpc_mem_chunk_alloc (urpc_server->sessions_chunks);
          if (session == NULL)
            {
              urpc_rwmutex_writer_unlock (&urpc_server->sessions_lock);
              status = URPC_STATUS_FAIL;
              goto urpc_server_send_reply;
            }

          session->state = URPC_STATE_GOT_SESSION_ID;
          session->sessions_chunks = urpc_server->sessions_chunks;
          session->socket = csocket;

          /* Запоминаем время подключения. */
          session->activity = urpc_timer_create ();
          if (session->activity == NULL)
            {
              urpc_rwmutex_writer_unlock (&urpc_server->sessions_lock);
              status = URPC_STATUS_FAIL;
              urpc_server_session_remove_func (session);
              session = NULL;
              goto urpc_server_send_reply;
            }

          /* Генерируем новый идентификатор. */
          session_id = urpc_server->last_session_id;
          do
            {
              session_id += 1;
            }
          while (session_id == 0 || urpc_hash_table_find (urpc_server->sessions, session_id) != NULL);

          /* Запоминаем сессию. */
          if (urpc_hash_table_insert (urpc_server->sessions, session_id, session) != 0)
            {
              urpc_rwmutex_writer_unlock (&urpc_server->sessions_lock);
              status = URPC_STATUS_FAIL;
              urpc_server_session_remove_func (session);
              session = NULL;
              goto urpc_server_send_reply;
            }

          /* Вызываем функцию при подключении клиента. */
          if (urpc_server->connect_proc != NULL)
            session->user_data = urpc_server->connect_proc (session_id, urpc_server->connect_proc_data, NULL);

          urpc_rwmutex_writer_unlock (&urpc_server->sessions_lock);

          status = URPC_STATUS_OK;
          goto urpc_server_send_reply;
        }

      /* Проверка наличия сессии. */
      urpc_rwmutex_reader_lock (&urpc_server->sessions_lock);
      session = urpc_hash_table_find (urpc_server->sessions, session_id);
      if (session == NULL)
        {
          urpc_rwmutex_reader_unlock (&urpc_server->sessions_lock);
          status = URPC_STATUS_AUTH_ERROR;
          goto urpc_server_send_reply;
        }
      urpc_timer_start (session->activity);
      urpc_rwmutex_reader_unlock (&urpc_server->sessions_lock);

      if (session->state == URPC_STATE_GOT_SESSION_ID)
        session->state = URPC_STATE_CONNECTED;

      /* Отключение клиента. */
      if (proc_id == URPC_PROC_LOGOUT && session->state == URPC_STATE_CONNECTED)
        {
          status = URPC_STATUS_OK;
          disconnect = URPC_TRUE;
          goto urpc_server_send_reply;
        }

      /* Вызов пользовательской функциии. */
      proc = urpc_hash_table_find (urpc_server->procs, proc_id);
      proc_data = urpc_hash_table_find (urpc_server->procs_data, proc_id);
      if (proc != NULL)
        {
          if (proc (urpc_data, thread_data, session->user_data, proc_data) == 0)
            status = URPC_STATUS_OK;
        }

      /* Ошибка при вызове пользовательской функции, отключаем клиента. */
      if (status != URPC_STATUS_OK)
        disconnect = URPC_TRUE;

      /* Отправка ответа. */
urpc_server_send_reply:
      urpc_data_set_uint32 (urpc_data, URPC_PARAM_STATUS, status);

      /* Заголовок отправляемого пакета. */
      send_size = URPC_HEADER_SIZE + urpc_data_get_data_size (urpc_data, URPC_DATA_OUTPUT);
      oheader->magic = UINT32_TO_BE (URPC_MAGIC);
      oheader->version = UINT32_TO_BE (URPC_VERSION);
      oheader->size = UINT32_TO_BE (send_size);
      oheader->session = UINT32_TO_BE (session_id);

      /* Отправка ответа. */
      switch (urpc_server->type)
        {
        case URPC_UDP:
          urpc_udp_server_send (urpc_server->transport, thread_id);
          break;

        case URPC_TCP:
          urpc_tcp_server_send (urpc_server->transport, thread_id);
          break;

        case URPC_SHM:
          urpc_shm_server_send (urpc_server->transport, thread_id);
          break;

        default:
          break;
        }

      /* Произошла ошибка или штатное отключение - удаляем сессию. */
      if (disconnect)
        {
          if (urpc_server->disconnect_proc != NULL)
            urpc_server->disconnect_proc (session->user_data, urpc_server->disconnect_proc_data);
          urpc_rwmutex_writer_lock (&urpc_server->sessions_lock);
          urpc_hash_table_remove (urpc_server->sessions, session_id);
          urpc_rwmutex_writer_unlock (&urpc_server->sessions_lock);

          /* Отключаем TCP/IP клиента. */
          if (urpc_server->type == URPC_TCP)
            {
              urpc_tcp_server_remove_client (urpc_server->transport, csocket);
              closesocket (csocket);
            }
        }

      /* Очищаем буферы приёма-передачи. */
      urpc_data_set_data_size (urpc_data, URPC_DATA_INPUT, 0);
      urpc_data_set_data_size (urpc_data, URPC_DATA_OUTPUT, 0);
    }

  /* Пользовательская функция остановки рабочего потока. */
  if (urpc_server->thread_stop_proc != NULL)
    urpc_server->thread_stop_proc (thread_data, urpc_server->thread_start_proc_data);

  /* Сигнализация о завершении потока. */
  urpc_mutex_lock (&urpc_server->lock);
  urpc_server->started_servers--;
  urpc_mutex_unlock (&urpc_server->lock);

  return NULL;
}

uRpcServer *
urpc_server_create (const char *uri,
                    uint32_t    threads_num,
                    uint32_t    max_clients,
                    double      session_timeout,
                    uint32_t    max_data_size,
                    double      data_timeout)
{
  uRpcServer *urpc_server = NULL;
  uRpcType urpc_type = URPC_UNKNOWN;

  unsigned int i;

  /* Инициализация сети. */
  if (!urpc_server_initialized)
    {
      urpc_network_init ();
      urpc_server_initialized = 1;
    }

  /* Проверка типа адреса. */
  urpc_type = urpc_get_type (uri);
  if (urpc_type == URPC_UNKNOWN)
    return NULL;

  /* Структура объекта. */
  urpc_server = malloc (sizeof (uRpcServer));
  if (urpc_server == NULL)
    return NULL;

  urpc_server->type = urpc_type;
  urpc_server->urpc_server_type = URPC_SERVER_TYPE;
  urpc_server->uri = NULL;
  urpc_server->connect_proc = NULL;
  urpc_server->connect_proc_data = NULL;
  urpc_server->disconnect_proc = NULL;
  urpc_server->disconnect_proc_data = NULL;
  urpc_server->procs = NULL;
  urpc_server->procs_data = NULL;
  urpc_server->sessions = NULL;
  urpc_server->sessions_chunks = NULL;
  urpc_server->last_session_id = 0;
  urpc_server->session_timeout = session_timeout;
  urpc_server->session_check = NULL;
  urpc_server->transport = NULL;
  urpc_server->servers = NULL;
  urpc_server->threads_num = threads_num;
  urpc_server->max_clients = max_clients;
  urpc_server->max_data_size = max_data_size;
  urpc_server->data_timeout = data_timeout;
  urpc_server->started_servers = 0;
  urpc_server->shutdown = 0;
  urpc_rwmutex_init (&urpc_server->sessions_lock);
  urpc_mutex_init (&urpc_server->lock);

  urpc_server->uri = malloc (strlen (uri) + 1);
  if (urpc_server->uri == NULL)
    goto urpc_server_create_fail;
  memcpy (urpc_server->uri, uri, strlen (uri) + 1);

  urpc_server->procs = urpc_hash_table_create (NULL);
  if (urpc_server->procs == NULL)
    goto urpc_server_create_fail;

  urpc_server->procs_data = urpc_hash_table_create (NULL);
  if (urpc_server->procs_data == NULL)
    goto urpc_server_create_fail;

  urpc_server->sessions = 
    urpc_hash_table_create ((urpc_hash_table_destroy_callback) urpc_server_session_remove_func);
  if (urpc_server->sessions == NULL)
    goto urpc_server_create_fail;

  urpc_server->sessions_chunks = urpc_mem_chunk_create (sizeof (uRpcServerSession));
  if (urpc_server->sessions_chunks == NULL)
    goto urpc_server_create_fail;

  urpc_server->servers = malloc (threads_num * sizeof (uRpcThread *));
  if (urpc_server->servers == NULL)
    goto urpc_server_create_fail;
  for (i = 0; i < threads_num; i++)
    urpc_server->servers[i] = NULL;

  return urpc_server;

urpc_server_create_fail:
  urpc_server_destroy (urpc_server);

  return NULL;
}

void
urpc_server_destroy (uRpcServer *urpc_server)
{
  uint32_t started_servers;
  unsigned int i;

  if (urpc_server->urpc_server_type != URPC_SERVER_TYPE)
    return;

  /* Сигнализируем потокам о завершении работы. */
  urpc_server->shutdown = 1;

  /* Ожидаем завершения работы всех потоков. */
  do
    {
      urpc_mutex_lock (&urpc_server->lock);
      started_servers = urpc_server->started_servers;
      urpc_mutex_unlock (&urpc_server->lock);
      urpc_timer_sleep (0.1);
    }
  while (started_servers != 0);

  /* Удаляем объекты потоков. */
  if (urpc_server->servers != NULL)
    {
      for (i = 0; i < urpc_server->threads_num; i++)
        {
          if (urpc_server->servers[i] != NULL)
            urpc_thread_destroy (urpc_server->servers[i]);
        }
    }

  if (urpc_server->session_check != NULL)
    urpc_thread_destroy (urpc_server->session_check);

  /* Удаляем объект обмена данными. */
  if (urpc_server->transport != NULL)
    {
      switch (urpc_server->type)
        {
        case URPC_UDP:
          urpc_udp_server_destroy (urpc_server->transport);
          break;

        case URPC_TCP:
          urpc_tcp_server_destroy (urpc_server->transport);
          break;

        case URPC_SHM:
          urpc_shm_server_destroy (urpc_server->transport);
          break;

        default:
          break;
        }
    }

  /* Удаляем объект. */
  if (urpc_server->servers != NULL)
    free (urpc_server->servers);
  if (urpc_server->procs_data != NULL)
    urpc_hash_table_destroy (urpc_server->procs_data);
  if (urpc_server->procs != NULL)
    urpc_hash_table_destroy (urpc_server->procs);
  if (urpc_server->sessions != NULL)
    urpc_hash_table_destroy (urpc_server->sessions);
  if (urpc_server->sessions_chunks != NULL)
    urpc_mem_chunk_destroy (urpc_server->sessions_chunks);
  if (urpc_server->uri != NULL)
    free (urpc_server->uri);

  urpc_mutex_clear (&urpc_server->lock);
  urpc_rwmutex_clear (&urpc_server->sessions_lock);

  free (urpc_server);
}

int
urpc_server_add_thread_start_callback (uRpcServer       *urpc_server,
                                       urpc_thread_proc  proc,
                                       void             *data)
{
  if (urpc_server->urpc_server_type != URPC_SERVER_TYPE)
    return -1;
  if (urpc_server->transport != NULL)
    return -1;

  urpc_server->thread_start_proc = proc;
  urpc_server->thread_start_proc_data = data;

  return 0;
}

int
urpc_server_add_thread_stop_callback (uRpcServer       *urpc_server,
                                      urpc_free_proc    proc,
                                      void             *data)
{
  if (urpc_server->urpc_server_type != URPC_SERVER_TYPE)
    return -1;
  if (urpc_server->transport != NULL)
    return -1;

  urpc_server->thread_stop_proc = proc;
  urpc_server->thread_stop_proc_data = data;

  return 0;
}

int
urpc_server_add_connect_callback (uRpcServer        *urpc_server,
                                  urpc_connect_proc  proc,
                                  void              *data)
{
  if (urpc_server->urpc_server_type != URPC_SERVER_TYPE)
    return -1;
  if (urpc_server->transport != NULL)
    return -1;

  urpc_server->connect_proc = proc;
  urpc_server->connect_proc_data = data;

  return 0;
}

int
urpc_server_add_disconnect_callback (uRpcServer     *urpc_server,
                                     urpc_free_proc  proc,
                                     void           *data)
{
  if (urpc_server->urpc_server_type != URPC_SERVER_TYPE)
    return -1;
  if (urpc_server->transport != NULL)
    return -1;

  urpc_server->disconnect_proc = proc;
  urpc_server->disconnect_proc_data = data;

  return 0;
}

int
urpc_server_add_callback (uRpcServer *urpc_server,
                          uint32_t    proc_id,
                          urpc_proc   func,
                          void       *data)
{
  if (urpc_server->urpc_server_type != URPC_SERVER_TYPE)
    return -1;
  if (urpc_server->transport != NULL)
    return -1;

  if (urpc_hash_table_find (urpc_server->procs, proc_id) != NULL)
    return -1;
  if (urpc_hash_table_insert (urpc_server->procs, proc_id, func) != 0)
    return -1;
  if (urpc_hash_table_insert (urpc_server->procs_data, proc_id, data) != 0)
    return -1;

  return 0;
}

int
urpc_server_bind (uRpcServer *urpc_server)
{
  uint32_t started_servers = 0;
  unsigned int i;

  if (urpc_server->urpc_server_type != URPC_SERVER_TYPE)
    return -1;

  /* Создаём транспортный объект. */
  switch (urpc_server->type)
    {
    case URPC_UDP:
      urpc_server->transport =
        urpc_udp_server_create (urpc_server->uri, urpc_server->threads_num, urpc_server->data_timeout);
      break;

    case URPC_TCP:
      urpc_server->transport =
        urpc_tcp_server_create (urpc_server->uri, urpc_server->threads_num, urpc_server->max_clients,
                                urpc_server->max_data_size, urpc_server->data_timeout);
      break;

    case URPC_SHM:
      urpc_server->transport =
        urpc_shm_server_create (urpc_server->uri, urpc_server->threads_num, urpc_server->max_data_size);
      break;

    default:
      return -1;
    }

  if (urpc_server->transport == NULL)
    return -1;

  /* Запускаем потоки обработки запросов. */
  for (i = 0; i < urpc_server->threads_num; i++)
    {
      urpc_server->servers[i] = urpc_thread_create (urpc_server_func, urpc_server);
      if (urpc_server->servers[i] == NULL)
        {
          urpc_server->shutdown = 1;
          return -1;
        }
    }

  /* Ожидаем начала работы всех потоков транспортных объектов. */
  do
    {
      urpc_mutex_lock (&urpc_server->lock);
      started_servers = urpc_server->started_servers;
      urpc_mutex_unlock (&urpc_server->lock);
      urpc_timer_sleep (0.1);
    }
  while (started_servers != urpc_server->threads_num);

  /* Запуск потока проверки сессий. */
  urpc_server->session_check = urpc_thread_create (urpc_server_session_timeout_check, urpc_server);
  if (urpc_server->session_check == NULL)
    {
      urpc_server->shutdown = 1;
      return -1;
    }

  /* Ожидаем начала работы потока проверки сессий. */
  do
    {
      urpc_mutex_lock (&urpc_server->lock);
      started_servers = urpc_server->started_servers;
      urpc_mutex_unlock (&urpc_server->lock);
      urpc_timer_sleep (0.1);
    }
  while (started_servers != urpc_server->threads_num + 1);

  return 0;
}
