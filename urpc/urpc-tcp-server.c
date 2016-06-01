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

#include "urpc-tcp-server.h"
#include "urpc-common.h"
#include "urpc-thread.h"
#include "urpc-rwmutex.h"
#include "urpc-network.h"
#include "urpc-timer.h"
#include "urpc-endian.h"

#include <stdlib.h>

#define URPC_TCP_SERVER_TYPE 0x53504354

struct _uRpcTCPServer
{
  uint32_t             urpc_tcp_server_type;   /* Тип объекта uRpcTCPServer. */

  SOCKET               lsocket;                /* Сокет входящих подключений клиентов. */

  SOCKET              *wsockets;               /* Рабочие сокеты подключенных клиентов. */
  SOCKET              *wsockets_per_threads;   /* Рабочие сокеты обслуживаемые потоками сервера. */

  uint32_t             buffer_size;            /* Размер буфера приёма-передачи. */
  uRpcData           **urpc_data;              /* Указатель на объекты RPC данных. */
  uint32_t             threads_num;            /* Число рабочих потоков. */
  uint32_t             max_clients;            /* Максимальное число подключенных клиентов. */
  uint32_t             cur_clients;            /* Текущее число подключений. */

  uRpcTimer          **timers;                 /* Таймаут таймеры. */
  double               timeout;                /* Таймаут обмена данными. */

  uRpcThread          *connector;              /* Поток обслуживания новых подключений. */
  volatile uint32_t    connector_status;       /* Признак запуска потока. */
  volatile uint32_t    shutdown;               /* Признак завершения работы. */
  uRpcRWMutex          lock;                   /* Блокировка доступа к критическим данным структуры. */
};

/* Функция обслуживания новых подключений в потоке. */
static void *
urpc_tcp_server_func (void *data)
{
  uRpcTCPServer *urpc_tcp_server = data;

  urpc_tcp_server->connector_status = 1;

  while (!urpc_tcp_server->shutdown)
    {
      fd_set sock_set;
      struct timeval sock_tv;
      int selected;

      SOCKET wsocket;
      unsigned int i;

      /* Ожидаем новых подключений клиентов в течение 100мс. */
      FD_ZERO (&sock_set);
      sock_tv.tv_sec = 0;
      sock_tv.tv_usec = 500000;
      FD_SET (urpc_tcp_server->lsocket, &sock_set);

      selected = select ((int) (urpc_tcp_server->lsocket + 1), &sock_set, NULL, NULL, &sock_tv);
      if (selected < 0)
        {
          if (urpc_network_last_error () == URPC_EINTR)
            return NULL;
          break;
        }
      if (selected == 0)
        continue;

      /* Достигнуто максимальное число клиентов. */
      if (urpc_tcp_server->cur_clients == urpc_tcp_server->max_clients)
        continue;

      /* Проверяем наличие новых клиентов. */
      wsocket = accept (urpc_tcp_server->lsocket, NULL, NULL);
      if (wsocket == INVALID_SOCKET)
        {
          int accept_error = urpc_network_last_error ();
          if (accept_error == URPC_EINTR || accept_error == URPC_EAGAIN)
            continue;
          break;
        }

      /* Для нового соединения устанавливаем не блокирующий режим работы,
         отключаем задержку при передаче данных и запоминаем его. */
      urpc_network_set_tcp_nodelay (wsocket);
      urpc_network_set_non_block (wsocket);

      urpc_rwmutex_writer_lock (&urpc_tcp_server->lock);
      for (i = 0; i < urpc_tcp_server->max_clients; i++)
        {
          if (urpc_tcp_server->wsockets[i] == INVALID_SOCKET)
            {
              urpc_tcp_server->wsockets[i] = wsocket;
              urpc_tcp_server->cur_clients += 1;
              break;
            }
        }
      urpc_rwmutex_writer_unlock (&urpc_tcp_server->lock);
    }

  urpc_tcp_server->connector_status = 0;

  return NULL;
}

uRpcTCPServer *
urpc_tcp_server_create (const char *uri,
                        uint32_t    threads_num,
                        uint32_t    max_clients,
                        uint32_t    max_data_size,
                        double      timeout)
{
  uRpcTCPServer *urpc_tcp_server = NULL;
  struct addrinfo *addr = NULL;
  unsigned int i;

  /* Проверка ограничений. */
  if (max_clients > FD_SETSIZE)
    return NULL;
  if (max_data_size > URPC_MAX_DATA_SIZE)
    return NULL;
  if (threads_num > URPC_MAX_THREADS_NUM)
    threads_num = URPC_MAX_THREADS_NUM;
  if (timeout < URPC_MIN_TIMEOUT)
    timeout = URPC_MIN_TIMEOUT;
  max_data_size += URPC_HEADER_SIZE;

  /* Проверяем тип адреса. */
  if (urpc_get_type (uri) != URPC_TCP)
    return NULL;

  /* Структура объекта. */
  urpc_tcp_server = malloc (sizeof (uRpcTCPServer));
  if (urpc_tcp_server == NULL)
    return NULL;

  urpc_tcp_server->urpc_tcp_server_type = URPC_TCP_SERVER_TYPE;
  urpc_tcp_server->lsocket = INVALID_SOCKET;
  urpc_tcp_server->wsockets = NULL;
  urpc_tcp_server->wsockets_per_threads = NULL;
  urpc_tcp_server->buffer_size = max_data_size;
  urpc_tcp_server->urpc_data = NULL;
  urpc_tcp_server->threads_num = threads_num;
  urpc_tcp_server->max_clients = max_clients;
  urpc_tcp_server->cur_clients = 0;
  urpc_tcp_server->timers = NULL;
  urpc_tcp_server->timeout = timeout;
  urpc_tcp_server->connector = NULL;
  urpc_tcp_server->connector_status = 0;
  urpc_tcp_server->shutdown = 0;
  urpc_rwmutex_init (&urpc_tcp_server->lock);

  /* Буферы приёма-передачи. */
  urpc_tcp_server->urpc_data = malloc (threads_num * sizeof (uRpcData *));
  if (urpc_tcp_server->urpc_data == NULL)
    goto urpc_tcp_server_create_fail;
  for (i = 0; i < threads_num; i++)
    urpc_tcp_server->urpc_data[i] = NULL;
  for (i = 0; i < threads_num; i++)
    {
      urpc_tcp_server->urpc_data[i] = urpc_data_create (max_data_size, sizeof (uRpcHeader), NULL, NULL, 0);
      if (urpc_tcp_server->urpc_data[i] == NULL)
        goto urpc_tcp_server_create_fail;
    }

  /* Таймаут таймеры. */
  urpc_tcp_server->timers = malloc (threads_num * sizeof (uRpcTimer *));
  if (urpc_tcp_server->timers == NULL)
    goto urpc_tcp_server_create_fail;
  for (i = 0; i < threads_num; i++)
    urpc_tcp_server->timers[i] = NULL;
  for (i = 0; i < threads_num; i++)
    {
      urpc_tcp_server->timers[i] = urpc_timer_create ();
      if (urpc_tcp_server->timers[i] == NULL)
        goto urpc_tcp_server_create_fail;
    }

  /* Массив рабочих сокетов обслуживаемых потоками. */
  urpc_tcp_server->wsockets = malloc (max_clients * sizeof (SOCKET));
  if (urpc_tcp_server->wsockets == NULL)
    goto urpc_tcp_server_create_fail;
  for (i = 0; i < max_clients; i++)
    urpc_tcp_server->wsockets[i] = INVALID_SOCKET;

  /* Массив сокетов подключенных клиентов. */
  urpc_tcp_server->wsockets_per_threads = malloc (threads_num * sizeof (SOCKET));
  if (urpc_tcp_server->wsockets_per_threads == NULL)
    goto urpc_tcp_server_create_fail;
  for (i = 0; i < threads_num; i++)
    urpc_tcp_server->wsockets_per_threads[i] = INVALID_SOCKET;

  /* Адрес сервера. */
  addr = urpc_get_sockaddr (uri);
  if (addr == NULL)
    goto urpc_tcp_server_create_fail;

  /* Сокет входящих подключений клиентов. */
  urpc_tcp_server->lsocket = socket (addr->ai_family, SOCK_STREAM, addr->ai_protocol);
  if (urpc_tcp_server->lsocket == INVALID_SOCKET)
    goto urpc_tcp_server_create_fail;
  urpc_network_set_reuse (urpc_tcp_server->lsocket);
  if (bind (urpc_tcp_server->lsocket, addr->ai_addr, (socklen_t) addr->ai_addrlen) < 0)
    goto urpc_tcp_server_create_fail;
  if (listen (urpc_tcp_server->lsocket, 5) < 0)
    goto urpc_tcp_server_create_fail;
  urpc_network_set_non_block (urpc_tcp_server->lsocket);

  /* Запускаем поток обслуживания новых подключений. */
  urpc_tcp_server->connector = urpc_thread_create (urpc_tcp_server_func, urpc_tcp_server);
  if (urpc_tcp_server->connector == NULL)
    goto urpc_tcp_server_create_fail;

  /* Ожидаем запуска потока. */
  while (urpc_tcp_server->connector_status == 0)
    urpc_timer_sleep (0.1);

  freeaddrinfo (addr);

  return urpc_tcp_server;

urpc_tcp_server_create_fail:
  urpc_tcp_server_destroy (urpc_tcp_server);
  if (addr != NULL)
    freeaddrinfo (addr);

  return NULL;
}

void
urpc_tcp_server_destroy (uRpcTCPServer *urpc_tcp_server)
{
  unsigned int i;

  if (urpc_tcp_server->urpc_tcp_server_type != URPC_TCP_SERVER_TYPE)
    return;

  /* Ожидаем завершение потока обработки подключений клиентов. */
  if (urpc_tcp_server->connector != NULL)
    {
      urpc_tcp_server->shutdown = 1;
      while (urpc_tcp_server->connector_status != 0)
        urpc_timer_sleep (0.1);
      urpc_thread_destroy (urpc_tcp_server->connector);
    }

  /* Закрываем сокет входящих подключений. */
  if (urpc_tcp_server->lsocket != INVALID_SOCKET)
    closesocket (urpc_tcp_server->lsocket);

  /* Удаляем таблицу подключенных клиентов (закрываем сокеты). */
  if (urpc_tcp_server->wsockets != NULL)
    {
      for (i = 0; i < urpc_tcp_server->max_clients; i++)
        {
          if (urpc_tcp_server->wsockets[i] != INVALID_SOCKET)
            closesocket (urpc_tcp_server->wsockets[i]);
        }
      free (urpc_tcp_server->wsockets);
    }

  if (urpc_tcp_server->wsockets_per_threads != NULL)
    free (urpc_tcp_server->wsockets_per_threads);

  /* Удаляем таймаут таймеры. */
  if (urpc_tcp_server->timers != NULL)
    {
      for (i = 0; i < urpc_tcp_server->threads_num; i++)
        {
          if (urpc_tcp_server->timers[i] != NULL)
            urpc_timer_destroy (urpc_tcp_server->timers[i]);
        }
      free (urpc_tcp_server->timers);
    }

  /* Освобождаем память буферов приёма-передачи. */
  if (urpc_tcp_server->urpc_data != NULL)
    {
      for (i = 0; i < urpc_tcp_server->threads_num; i++)
        {
          if (urpc_tcp_server->urpc_data[i] != NULL)
            urpc_data_destroy (urpc_tcp_server->urpc_data[i]);
        }
      free (urpc_tcp_server->urpc_data);
    }

  free (urpc_tcp_server);
}

uRpcData *
urpc_tcp_server_recv (uRpcTCPServer *urpc_tcp_server,
                      uint32_t       thread_id)
{
  fd_set sock_set;
  struct timeval sock_tv;

  uRpcTimer *timer;
  uRpcData *urpc_data;
  uRpcHeader *iheader;

  int selected;
  unsigned int recv_size;
  unsigned int received = 0;
  int sr_size;

  SOCKET max_fd = 0;
  SOCKET wsocket = INVALID_SOCKET;

  unsigned int i, j;

  if (urpc_tcp_server->urpc_tcp_server_type != URPC_TCP_SERVER_TYPE)
    return NULL;
  if (thread_id > urpc_tcp_server->threads_num - 1)
    return NULL;

  /* Ожидаем запрос от клиента в течение 500мс. */
  FD_ZERO (&sock_set);
  sock_tv.tv_sec = 0;
  sock_tv.tv_usec = 500000;

  /* Список рабочих сокетов. */
  urpc_rwmutex_reader_lock (&urpc_tcp_server->lock);
  if (urpc_tcp_server->cur_clients == 0)
    {
      FD_SET (urpc_tcp_server->lsocket, &sock_set);
      max_fd = urpc_tcp_server->lsocket;
    }
  else
    {
      for (i = 0; i < urpc_tcp_server->max_clients; i++)
        {
          if (urpc_tcp_server->wsockets[i] != INVALID_SOCKET)
            {
              FD_SET (urpc_tcp_server->wsockets[i], &sock_set);
              if (urpc_tcp_server->wsockets[i] > max_fd)
                max_fd = urpc_tcp_server->wsockets[i];
            }
        }
    }
  urpc_rwmutex_reader_unlock (&urpc_tcp_server->lock);

  /* Ожидаем запрос. */
  selected = select ((int) (max_fd + 1), &sock_set, NULL, NULL, &sock_tv);
  if (selected < 0)
    {
      if (urpc_network_last_error () == URPC_EINTR)
        return NULL;
      return NULL;
    }
  if (selected == 0)
    return NULL;

  /* Смотрим какой из клиентов прислал запрос. */
  urpc_rwmutex_writer_lock (&urpc_tcp_server->lock);
  urpc_tcp_server->wsockets_per_threads[thread_id] = INVALID_SOCKET;
  for (i = 0; i < urpc_tcp_server->max_clients; i++)
    {
      wsocket = urpc_tcp_server->wsockets[i];
      if (FD_ISSET (wsocket, &sock_set))
        {
          /* Проверяем, что этот сокет не обслуживается в другом потоке. */
          for (j = 0; j < urpc_tcp_server->threads_num; j++)
            {
              if (urpc_tcp_server->wsockets_per_threads[j] == wsocket)
                break;
            }
          if (j == urpc_tcp_server->threads_num)
            {
              urpc_tcp_server->wsockets_per_threads[thread_id] = wsocket;
              break;
            }
          continue;
        }
    }
  urpc_rwmutex_writer_unlock (&urpc_tcp_server->lock);

  /* Нет запросов. */
  if (urpc_tcp_server->wsockets_per_threads[thread_id] == INVALID_SOCKET)
    return NULL;

  timer = urpc_tcp_server->timers[thread_id];
  urpc_data = urpc_tcp_server->urpc_data[thread_id];
  iheader = urpc_data_get_header (urpc_data, URPC_DATA_INPUT);

  /* Время начала приёма. */
  urpc_timer_start (timer);

  /* Принимаем заголовок запроса. */
  recv_size = sizeof (uRpcHeader);
  while (received != recv_size)
    {

      /* Проверка таймаута при приёме данных. */
      if (urpc_timer_elapsed (timer) > urpc_tcp_server->timeout)
        {
          urpc_tcp_server_disconnect_client (urpc_tcp_server, wsocket);
          return NULL;
        }

      /* Проверяем возможность чтения из канала связи с интервалом в 100мс. */
      FD_ZERO (&sock_set);
      FD_SET (wsocket, &sock_set);
      sock_tv.tv_sec = 0;
      sock_tv.tv_usec = 100000;

      selected = select ((int) (wsocket + 1), &sock_set, NULL, NULL, &sock_tv);
      if (selected < 0)
        {
          if (urpc_network_last_error () == URPC_EINTR)
            continue;
          urpc_tcp_server_disconnect_client (urpc_tcp_server, wsocket);
          return NULL;
        }

      if (selected == 0)
        continue;

      /* Считываем данные. */
      sr_size = recv (wsocket, (char *) iheader + received, recv_size - received, URPC_MSG_NOSIGNAL);
      if (sr_size <= 0)
        {
          int recv_error = urpc_network_last_error ();
          if (recv_error == URPC_EINTR || recv_error == URPC_EAGAIN)
            continue;
          urpc_tcp_server_disconnect_client (urpc_tcp_server, wsocket);
          return NULL;
        }

      received += sr_size;

      /* Перезапускаем таймер. */
      urpc_timer_start (timer);
    }

  /* Проверяем заголовок запроса. */
  if (UINT32_FROM_BE (iheader->magic) != URPC_MAGIC)
    {
      urpc_tcp_server_disconnect_client (urpc_tcp_server, wsocket);
      return NULL;
    }
  recv_size = UINT32_FROM_BE (iheader->size);
  if (recv_size > urpc_tcp_server->buffer_size)
    {
      urpc_tcp_server_disconnect_client (urpc_tcp_server, wsocket);
      return NULL;
    }

  /* Принимаем данные запроса. */
  while (received != recv_size)
    {
      /* Проверка таймаута при приёме данных. */
      if (urpc_timer_elapsed (timer) > urpc_tcp_server->timeout)
        {
          urpc_tcp_server_disconnect_client (urpc_tcp_server, wsocket);
          return NULL;
        }

      /* Проверяем возможность чтения из канала связи с интервалом в 100мс. */
      FD_ZERO (&sock_set);
      FD_SET (wsocket, &sock_set);
      sock_tv.tv_sec = 0;
      sock_tv.tv_usec = 100000;

      selected = select ((int) (wsocket + 1), &sock_set, NULL, NULL, &sock_tv);
      if (selected < 0)
        {
          if (urpc_network_last_error () == URPC_EINTR)
            continue;
          urpc_tcp_server_disconnect_client (urpc_tcp_server, wsocket);
          return NULL;
        }

      if (selected == 0)
        continue;

      /* Считываем данные. */
      sr_size = recv (wsocket, (char *) iheader + received, recv_size - received, URPC_MSG_NOSIGNAL);
      if (sr_size <= 0)
        {
          int recv_error = urpc_network_last_error ();
          if (recv_error == URPC_EINTR || recv_error == URPC_EAGAIN)
            continue;
          urpc_tcp_server_disconnect_client (urpc_tcp_server, wsocket);
          return NULL;
        }

      received += sr_size;

      /* Перезапускаем таймер. */
      urpc_timer_start (timer);
    }

  urpc_data_set_data_size (urpc_data, URPC_DATA_INPUT, recv_size - URPC_HEADER_SIZE);

  return urpc_data;
}

int
urpc_tcp_server_send (uRpcTCPServer *urpc_tcp_server,
                      uint32_t       thread_id)
{
  uRpcData *urpc_data;
  uRpcHeader *oheader;

  fd_set sock_set;
  struct timeval sock_tv;

  uRpcTimer *timer;
  SOCKET wsocket;

  int selected;
  unsigned int send_size;
  unsigned int sended = 0;
  int sr_size;

  if (urpc_tcp_server->urpc_tcp_server_type != URPC_TCP_SERVER_TYPE)
    return -1;
  if (thread_id > urpc_tcp_server->threads_num - 1)
    return -1;

  wsocket = urpc_tcp_server->wsockets_per_threads[thread_id];
  urpc_tcp_server->wsockets_per_threads[thread_id] = INVALID_SOCKET;
  if (wsocket == INVALID_SOCKET)
    return -1;

  timer = urpc_tcp_server->timers[thread_id];

  /* Отправляемые данные. */
  urpc_data = urpc_tcp_server->urpc_data[thread_id];
  oheader = urpc_data_get_header (urpc_data, URPC_DATA_OUTPUT);
  send_size = UINT32_FROM_BE (oheader->size);

  /* Время начала передачи. */
  urpc_timer_start (timer);

  /* Отправляем ответ. */
  while (sended != send_size)
    {
      /* Проверка таймаута при передаче данных. */
      if (urpc_timer_elapsed (timer) > urpc_tcp_server->timeout)
        {
          urpc_tcp_server_disconnect_client (urpc_tcp_server, wsocket);
          return -1;
        }

      /* Проверяем возможность записи в канал связи с интервалом в 100мс. */
      FD_ZERO (&sock_set);
      FD_SET (wsocket, &sock_set);
      sock_tv.tv_sec = 0;
      sock_tv.tv_usec = 100000;

      selected = select ((int) (wsocket + 1), NULL, &sock_set, NULL, &sock_tv);
      if (selected < 0)
        {
          if (urpc_network_last_error () == URPC_EINTR)
            continue;
          urpc_tcp_server_disconnect_client (urpc_tcp_server, wsocket);
          return -1;
        }

      if (selected == 0)
        continue;

      /* Отправляем данные. */
      sr_size = send (wsocket, (char *) oheader + sended, send_size - sended, URPC_MSG_NOSIGNAL);
      if (sr_size <= 0)
        {
          int send_error = urpc_network_last_error ();
          if (send_error == URPC_EINTR || send_error == URPC_EAGAIN)
            continue;
          urpc_tcp_server_disconnect_client (urpc_tcp_server, wsocket);
          return -1;
        }

      sended += sr_size;

      /* Перезапускаем таймер. */
      urpc_timer_start (timer);
    }

  return 0;
}

SOCKET
urpc_tcp_server_get_client_socket (uRpcTCPServer *urpc_tcp_server,
                                   uint32_t       thread_id)
{
  if (urpc_tcp_server->urpc_tcp_server_type != URPC_TCP_SERVER_TYPE)
    return INVALID_SOCKET;
  if (thread_id > urpc_tcp_server->threads_num - 1)
    return INVALID_SOCKET;

  /* Возвращаем сокет текущего клиента обрабатываемого потоком. */
  return urpc_tcp_server->wsockets_per_threads[thread_id];
}

int
urpc_tcp_server_disconnect_client (uRpcTCPServer *urpc_tcp_server,
                                   SOCKET         wsocket)
{
  unsigned int i;

  if (urpc_tcp_server->urpc_tcp_server_type != URPC_TCP_SERVER_TYPE)
    return -1;

  /* Закрываем сокет указанного клиента и удаляем его из списка. */
  urpc_rwmutex_writer_lock (&urpc_tcp_server->lock);
  for (i = 0; i < urpc_tcp_server->max_clients; i++)
    {
      if (urpc_tcp_server->wsockets[i] == wsocket)
        {
          closesocket (urpc_tcp_server->wsockets[i]);
          urpc_tcp_server->wsockets[i] = INVALID_SOCKET;
          urpc_tcp_server->cur_clients -= 1;
          break;
        }
    }
  urpc_rwmutex_writer_unlock (&urpc_tcp_server->lock);

  return 0;
}
