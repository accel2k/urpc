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

#include "urpc-tcp-client.h"
#include "urpc-common.h"
#include "urpc-network.h"
#include "urpc-timer.h"
#include "urpc-endian.h"

#include <stdio.h>
#include <stdlib.h>

#if defined _MSVC_COMPILER
#define snprintf sprintf_s
#endif

#define URPC_TCP_CLIENT_TYPE 0x43504354

#define TCP_ADDRESS_SIZE     1024
#define TCP_PORT_SIZE        64
#define TCP_PAD_SIZE         64
#define TCP_INFO_SIZE        TCP_ADDRESS_SIZE + TCP_PORT_SIZE + TCP_PAD_SIZE

struct _uRpcTCPClient
{
  uint32_t             urpc_tcp_client_type;   /* Тип объекта uRpcTCPClient. */

  SOCKET               socket;                 /* Рабочий сокет. */

  uint32_t             buffer_size;            /* Размер буфера приёма-передачи. */
  uRpcData            *urpc_data;              /* Указатель на объект RPC данных. */
  uRpcTimer           *timer;                  /* Таймаут таймер. */
  double               timeout;                /* Интервал таймаута. */

  char                *self_address;           /* Локальный адрес. */
  char                *peer_address;           /* Адрес сервера. */

  volatile uint32_t    fail;                   /* Признак ошибки. */
};

uRpcTCPClient *
urpc_tcp_client_create (const char *uri,
                        uint32_t    max_data_size,
                        double      timeout)
{
  uRpcTCPClient *urpc_tcp_client = NULL;
  struct addrinfo *addr = NULL;

  struct sockaddr self_addr;
  socklen_t self_addr_size;

  char ips[TCP_ADDRESS_SIZE];
  char ports[TCP_PORT_SIZE];

  /* Проверка ограничений. */
  if (max_data_size > URPC_MAX_DATA_SIZE)
    return NULL;
  if (timeout < URPC_MIN_TIMEOUT)
    timeout = URPC_MIN_TIMEOUT;
  max_data_size += URPC_HEADER_SIZE;

  /* Проверяем тип адреса. */
  if (urpc_get_type (uri) != URPC_TCP)
    return NULL;

  /* Структура объекта. */
  urpc_tcp_client = malloc (sizeof (uRpcTCPClient));
  if (urpc_tcp_client == NULL)
    return NULL;

  urpc_tcp_client->urpc_tcp_client_type = URPC_TCP_CLIENT_TYPE;
  urpc_tcp_client->socket = INVALID_SOCKET;
  urpc_tcp_client->buffer_size = max_data_size;
  urpc_tcp_client->urpc_data = NULL;
  urpc_tcp_client->timer = NULL;
  urpc_tcp_client->timeout = timeout;
  urpc_tcp_client->self_address = NULL;
  urpc_tcp_client->peer_address = NULL;
  urpc_tcp_client->fail = 0;

  /* Буферы приёма-передачи. */
  urpc_tcp_client->urpc_data = urpc_data_create (max_data_size, sizeof (uRpcHeader), NULL, NULL, 0);
  if (urpc_tcp_client->urpc_data == NULL)
    goto urpc_tcp_client_create_fail;

  /* Адрес сервера. */
  addr = urpc_get_sockaddr (uri);
  if (addr == NULL)
    goto urpc_tcp_client_create_fail;

  /* Рабочий сокет. */
  urpc_tcp_client->socket = socket (addr->ai_family, SOCK_STREAM, addr->ai_protocol);
  if (urpc_tcp_client->socket == INVALID_SOCKET)
    goto urpc_tcp_client_create_fail;
  if (connect (urpc_tcp_client->socket, addr->ai_addr, (socklen_t) addr->ai_addrlen) < 0)
    goto urpc_tcp_client_create_fail;
  urpc_network_set_tcp_nodelay (urpc_tcp_client->socket);
  urpc_network_set_non_block (urpc_tcp_client->socket);
  /* Локальный адрес. */
  urpc_tcp_client->self_address = malloc (sizeof (ips) + sizeof (ports));
  if (urpc_tcp_client->self_address == NULL)
    goto urpc_tcp_client_create_fail;

  self_addr_size = sizeof (self_addr);
  if (getsockname (urpc_tcp_client->socket, &self_addr, &self_addr_size) != 0)
    goto urpc_tcp_client_create_fail;

  if (getnameinfo (&self_addr, self_addr_size,
                   ips, sizeof (ips),
                   ports, sizeof (ports),
                   NI_NUMERICHOST | NI_NUMERICSERV) != 0)
    {
      goto urpc_tcp_client_create_fail;
    }

  if (self_addr.sa_family == AF_INET)
    {
      snprintf (urpc_tcp_client->self_address, TCP_INFO_SIZE,
                "tcp://%s:%s",
                ips, ports);
    }
  else if (self_addr.sa_family == AF_INET6)
    {
      snprintf (urpc_tcp_client->self_address, TCP_INFO_SIZE,
                "tcp://[%s]:%s",
                ips, ports);
    }
  else
    {
      goto urpc_tcp_client_create_fail;
    }

  /* Адрес сервера. */
  urpc_tcp_client->peer_address = malloc (sizeof (ips) + sizeof (ports));
  if (urpc_tcp_client->peer_address == NULL)
    goto urpc_tcp_client_create_fail;

  if (getnameinfo (addr->ai_addr, (socklen_t) addr->ai_addrlen,
                   ips, sizeof (ips),
                   ports, sizeof (ports),
                   NI_NUMERICHOST | NI_NUMERICSERV) != 0)
    {
      goto urpc_tcp_client_create_fail;
    }

  if (addr->ai_addr->sa_family == AF_INET)
    {
      snprintf (urpc_tcp_client->peer_address, TCP_INFO_SIZE,
                "tcp://%s:%s",
                ips, ports);
    }
  else if (addr->ai_addr->sa_family == AF_INET6)
    {
      snprintf (urpc_tcp_client->peer_address, TCP_INFO_SIZE,
                "tcp://[%s]:%s",
                ips, ports);
    }
  else
    {
      goto urpc_tcp_client_create_fail;
    }

  /* Таймер передачи. */
  urpc_tcp_client->timer = urpc_timer_create ();
  if (urpc_tcp_client->timer == NULL)
    goto urpc_tcp_client_create_fail;

  freeaddrinfo (addr);

  return urpc_tcp_client;

urpc_tcp_client_create_fail:
  urpc_tcp_client_destroy (urpc_tcp_client);
  if (addr != NULL)
    freeaddrinfo (addr);

  return NULL;
}

void
urpc_tcp_client_destroy (uRpcTCPClient *urpc_tcp_client)
{
  if (urpc_tcp_client->urpc_tcp_client_type != URPC_TCP_CLIENT_TYPE)
    return;

  if (urpc_tcp_client->socket != INVALID_SOCKET)
    closesocket (urpc_tcp_client->socket);
  if (urpc_tcp_client->timer != NULL)
    urpc_timer_destroy (urpc_tcp_client->timer);
  if (urpc_tcp_client->urpc_data != NULL)
    urpc_data_destroy (urpc_tcp_client->urpc_data);

  if (urpc_tcp_client->self_address != NULL)
    free (urpc_tcp_client->self_address);
  if (urpc_tcp_client->peer_address != NULL)
    free (urpc_tcp_client->peer_address);

  free (urpc_tcp_client);
}

uRpcData *
urpc_tcp_client_lock (uRpcTCPClient *urpc_tcp_client)
{
  if (urpc_tcp_client->urpc_tcp_client_type != URPC_TCP_CLIENT_TYPE)
    return NULL;
  if (urpc_tcp_client->fail)
    return NULL;

  return urpc_tcp_client->urpc_data;
}

uint32_t
urpc_tcp_client_exchange (uRpcTCPClient *urpc_tcp_client)
{
  fd_set sock_set;
  struct timeval sock_tv;

  uRpcHeader *iheader;
  uRpcHeader *oheader;

  unsigned int send_size;
  unsigned int recv_size;

  int selected;
  unsigned int sended = 0;
  unsigned int received = 0;

  int sr_size;

  if (urpc_tcp_client->urpc_tcp_client_type != URPC_TCP_CLIENT_TYPE)
    return URPC_STATUS_FAIL;
  if (urpc_tcp_client->fail)
    return URPC_STATUS_TRANSPORT_ERROR;

  iheader = urpc_data_get_header (urpc_tcp_client->urpc_data, URPC_DATA_INPUT);
  oheader = urpc_data_get_header (urpc_tcp_client->urpc_data, URPC_DATA_OUTPUT);
  send_size = UINT32_FROM_BE (oheader->size);

  /* Время начала передачи. */
  urpc_timer_start (urpc_tcp_client->timer);

  /* Отправляем запрос. */
  while (sended != send_size)
    {
      /* Проверка таймаута при передаче данных. */
      if (urpc_timer_elapsed (urpc_tcp_client->timer) > urpc_tcp_client->timeout)
        {
          urpc_tcp_client->fail = 1;
          return URPC_STATUS_TRANSPORT_ERROR;
        }

      /* Проверяем возможность записи в канал связи с интервалом в 100мс. */
      FD_ZERO (&sock_set);
      FD_SET (urpc_tcp_client->socket, &sock_set);
      sock_tv.tv_sec = 0;
      sock_tv.tv_usec = 100000;

      selected = select ((int) (urpc_tcp_client->socket + 1), NULL, &sock_set, NULL, &sock_tv);
      if (selected < 0)
        {
          int error = urpc_network_last_error ();
          if (error == URPC_EINTR)
            continue;
          urpc_tcp_client->fail = 1;
          return URPC_STATUS_TRANSPORT_ERROR;
        }

      if (selected == 0)
        continue;

      /* Отправляем данные. */
      sr_size = send (urpc_tcp_client->socket, (char *) oheader + sended, send_size - sended, URPC_MSG_NOSIGNAL);
      if (sr_size <= 0)
        {
          int error = urpc_network_last_error ();
          if (error == URPC_EINTR || error == URPC_EAGAIN)
            continue;
          urpc_tcp_client->fail = 1;
          return URPC_STATUS_TRANSPORT_ERROR;
        }

      sended += sr_size;

      /* Перезапускаем таймер. */
      urpc_timer_start (urpc_tcp_client->timer);
    }

  /* Время начала приёма. */
  urpc_timer_start (urpc_tcp_client->timer);

  /* Принимаем заголовок ответа. */
  recv_size = sizeof (uRpcHeader);
  while (received != recv_size)
    {
      /* Проверка таймаута при приёме данных. */
      if (urpc_timer_elapsed (urpc_tcp_client->timer) > urpc_tcp_client->timeout)
        {
          urpc_tcp_client->fail = 1;
          return URPC_STATUS_TRANSPORT_ERROR;
        }

      /* Проверяем возможность чтения из канала связи с интервалом в 100мс. */
      FD_ZERO (&sock_set);
      FD_SET (urpc_tcp_client->socket, &sock_set);
      sock_tv.tv_sec = 0;
      sock_tv.tv_usec = 100000;

      selected = select ((int) (urpc_tcp_client->socket + 1), &sock_set, NULL, NULL, &sock_tv);
      if (selected < 0)
        {
          int error = urpc_network_last_error ();
          if (error == URPC_EINTR)
            continue;
          urpc_tcp_client->fail = 1;
          return URPC_STATUS_TRANSPORT_ERROR;
        }

      if (selected == 0)
        continue;

      /* Считываем данные. */
      sr_size =  recv (urpc_tcp_client->socket, (char *) iheader + received, 
                       recv_size - received, URPC_MSG_NOSIGNAL);
      if (sr_size <= 0)
        {
          int error = urpc_network_last_error ();
          if (error == URPC_EINTR || error == URPC_EAGAIN)
            continue;
          urpc_tcp_client->fail = 1;
          return URPC_STATUS_TRANSPORT_ERROR;
        }

      received += sr_size;

      /* Перезапускаем таймер. */
      urpc_timer_start (urpc_tcp_client->timer);
    }

  /* Проверяем заголовок ответа. */
  if (UINT32_FROM_BE (iheader->magic) != URPC_MAGIC)
    {
      urpc_tcp_client->fail = 1;
      return URPC_STATUS_TRANSPORT_ERROR;
    }
  recv_size = UINT32_FROM_BE (iheader->size);
  if (recv_size > urpc_tcp_client->buffer_size)
    {
      urpc_tcp_client->fail = 1;
      return URPC_STATUS_TRANSPORT_ERROR;
    }

  /* Принимаем данные ответа. */
  while (received != recv_size)
    {
      /* Проверка таймаута при приёме данных. */
      if (urpc_timer_elapsed (urpc_tcp_client->timer) > urpc_tcp_client->timeout)
        {
          urpc_tcp_client->fail = 1;
          return URPC_STATUS_TRANSPORT_ERROR;
        }

      /* Проверяем возможность чтения из канала связи с интервалом в 100мс. */
      FD_ZERO (&sock_set);
      FD_SET (urpc_tcp_client->socket, &sock_set);
      sock_tv.tv_sec = 0;
      sock_tv.tv_usec = 100000;

      selected = select ((int) (urpc_tcp_client->socket + 1), &sock_set, NULL, NULL, &sock_tv);
      if (selected < 0)
        {
          int error = urpc_network_last_error ();
          if (error == URPC_EINTR)
            continue;
          urpc_tcp_client->fail = 1;
          return URPC_STATUS_TRANSPORT_ERROR;
        }

      if (selected == 0)
        continue;

      /* Считываем данные. */
      sr_size = recv (urpc_tcp_client->socket, (char *) iheader + received,
                      recv_size - received, URPC_MSG_NOSIGNAL);
      if (sr_size <= 0)
        {
          int error = urpc_network_last_error ();
          if (error == URPC_EINTR || error == URPC_EAGAIN)
            continue;
          urpc_tcp_client->fail = 1;
          return URPC_STATUS_TRANSPORT_ERROR;
        }

      received += sr_size;

      /* Перезапускаем таймер. */
      urpc_timer_start (urpc_tcp_client->timer);
    }

  urpc_data_set_data_size (urpc_tcp_client->urpc_data, URPC_DATA_INPUT, recv_size - URPC_HEADER_SIZE);

  return URPC_STATUS_OK;
}

const char *
urpc_tcp_client_get_self_address (uRpcTCPClient *urpc_tcp_client)
{
  if (urpc_tcp_client->urpc_tcp_client_type != URPC_TCP_CLIENT_TYPE)
    return NULL;
  if (urpc_tcp_client->fail)
    return NULL;

  return urpc_tcp_client->self_address;
}

const char *
urpc_tcp_client_get_peer_address (uRpcTCPClient *urpc_tcp_client)
{
  if (urpc_tcp_client->urpc_tcp_client_type != URPC_TCP_CLIENT_TYPE)
    return NULL;
  if (urpc_tcp_client->fail)
    return NULL;

  return urpc_tcp_client->peer_address;
}
