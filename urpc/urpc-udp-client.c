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

#include "urpc-udp-client.h"
#include "urpc-common.h"
#include "urpc-network.h"
#include "urpc-timer.h"
#include "urpc-endian.h"

#include <stdio.h>
#include <stdlib.h>

#if defined _MSVC_COMPILER
#define snprintf sprintf_s
#endif

#define URPC_UDP_CLIENT_TYPE 0x43504455

struct _uRpcUDPClient
{
  uint32_t             urpc_udp_client_type;   /* Тип объекта uRpcUDPClient. */

  SOCKET               socket;                 /* Рабочий сокет. */

  uRpcData            *urpc_data;              /* Указатель на объект RPC данных. */
  uRpcTimer           *timer;                  /* Таймаут таймер. */
  double               timeout;                /* Таймаут обмена данными. */

  char                *self_address;           /* Локальный адрес. */
  char                *peer_address;           /* Адрес сервера. */

  volatile uint32_t    fail;                   /* Признак ошибки. */
};

uRpcUDPClient *
urpc_udp_client_create (const char *uri,
                        double      timeout)
{
  uRpcUDPClient *urpc_udp_client = NULL;
  struct addrinfo *addr = NULL;

  struct sockaddr self_addr;
  socklen_t self_addr_size;

  char ips[1024];
  char ports[64];

  /* Проверка ограничений. */
  if (timeout < URPC_MIN_TIMEOUT)
    timeout = URPC_MIN_TIMEOUT;

  /* Проверяем тип адреса. */
  if (urpc_get_type (uri) != URPC_UDP)
    return NULL;

  /* Структура объекта. */
  urpc_udp_client = malloc (sizeof (uRpcUDPClient));
  if (urpc_udp_client == NULL)
    return NULL;

  urpc_udp_client->urpc_udp_client_type = URPC_UDP_CLIENT_TYPE;
  urpc_udp_client->socket = INVALID_SOCKET;
  urpc_udp_client->urpc_data = NULL;
  urpc_udp_client->timer = NULL;
  urpc_udp_client->timeout = timeout;
  urpc_udp_client->self_address = NULL;
  urpc_udp_client->peer_address = NULL;
  urpc_udp_client->fail = 0;

  /* Буферы приёма-передачи. */
  urpc_udp_client->urpc_data = urpc_data_create (URPC_DEFAULT_BUFFER_SIZE, sizeof (uRpcHeader),
                                                 NULL, NULL, 0);
  if (urpc_udp_client->urpc_data == NULL)
    goto urpc_udp_client_create_fail;

  /* Адрес сервера. */
  addr = urpc_get_sockaddr (uri);
  if (addr == NULL)
    goto urpc_udp_client_create_fail;

  /* Рабочий сокет. */
  urpc_udp_client->socket = socket (addr->ai_family, SOCK_DGRAM, addr->ai_protocol);
  if (urpc_udp_client->socket == INVALID_SOCKET)
    goto urpc_udp_client_create_fail;
  if (connect (urpc_udp_client->socket, addr->ai_addr, (socklen_t) addr->ai_addrlen) != 0)
    goto urpc_udp_client_create_fail;
  urpc_network_set_non_block (urpc_udp_client->socket);

  /* Локальный адрес. */
  urpc_udp_client->self_address = malloc (sizeof (ips) + sizeof (ports));
  if (urpc_udp_client->self_address == NULL)
    goto urpc_udp_client_create_fail;

  self_addr_size = sizeof (self_addr);
  if (getsockname (urpc_udp_client->socket, &self_addr, &self_addr_size) != 0)
    goto urpc_udp_client_create_fail;

  if (getnameinfo (&self_addr, self_addr_size,
                   ips, sizeof (ips),
                   ports, sizeof (ports),
                   NI_NUMERICHOST | NI_NUMERICSERV) != 0)
    {
      goto urpc_udp_client_create_fail;
    }

  if (self_addr.sa_family == AF_INET)
    {
      snprintf (urpc_udp_client->self_address, sizeof (ips) + sizeof (ports),
                "udp://%s:%s",
                ips, ports);
    }
  else if (self_addr.sa_family == AF_INET6)
    {
      snprintf (urpc_udp_client->self_address, sizeof (ips) + sizeof (ports),
                "udp://[%s]:%s",
                ips, ports);
    }
  else
    {
      goto urpc_udp_client_create_fail;
    }

  /* Адрес сервера. */
  urpc_udp_client->peer_address = malloc (sizeof (ips) + sizeof (ports));
  if (urpc_udp_client->peer_address == NULL)
    goto urpc_udp_client_create_fail;

  if (getnameinfo (addr->ai_addr, (socklen_t) addr->ai_addrlen,
                   ips, sizeof (ips),
                   ports, sizeof (ports),
                   NI_NUMERICHOST | NI_NUMERICSERV) != 0)
    {
      goto urpc_udp_client_create_fail;
    }

  if (addr->ai_addr->sa_family == AF_INET)
    {
      snprintf (urpc_udp_client->peer_address, sizeof (ips) + sizeof (ports),
                "udp://%s:%s",
                ips, ports);
    }
  else if (addr->ai_addr->sa_family == AF_INET6)
    {
      snprintf (urpc_udp_client->peer_address, sizeof (ips) + sizeof (ports),
                "udp://[%s]:%s",
                ips, ports);
    }
  else
    {
      goto urpc_udp_client_create_fail;
    }

  /* Таймер передачи. */
  urpc_udp_client->timer = urpc_timer_create ();
  if (urpc_udp_client->timer == NULL)
    goto urpc_udp_client_create_fail;

  freeaddrinfo (addr);

  return urpc_udp_client;

urpc_udp_client_create_fail:
  urpc_udp_client_destroy (urpc_udp_client);
  if (addr != NULL)
    freeaddrinfo (addr);

  return NULL;
}


void
urpc_udp_client_destroy (uRpcUDPClient *urpc_udp_client)
{
  if (urpc_udp_client->urpc_udp_client_type != URPC_UDP_CLIENT_TYPE)
    return;

  if (urpc_udp_client->socket != INVALID_SOCKET)
    closesocket (urpc_udp_client->socket);
  if (urpc_udp_client->timer != NULL)
    urpc_timer_destroy (urpc_udp_client->timer);
  if (urpc_udp_client->urpc_data != NULL)
    urpc_data_destroy (urpc_udp_client->urpc_data);

  if (urpc_udp_client->self_address != NULL)
    free (urpc_udp_client->self_address);
  if (urpc_udp_client->peer_address != NULL)
    free (urpc_udp_client->peer_address);

  free (urpc_udp_client);
}

uRpcData *
urpc_udp_client_lock (uRpcUDPClient *urpc_udp_client)
{
  if (urpc_udp_client->urpc_udp_client_type != URPC_UDP_CLIENT_TYPE)
    return NULL;
  if (urpc_udp_client->fail)
    return NULL;

  return urpc_udp_client->urpc_data;
}

uint32_t
urpc_udp_client_exchange (uRpcUDPClient *urpc_udp_client)
{
  fd_set sock_set;
  struct timeval sock_tv;

  uRpcHeader *iheader;
  uRpcHeader *oheader;

  int recv_size;

  if (urpc_udp_client->urpc_udp_client_type != URPC_UDP_CLIENT_TYPE)
    return URPC_STATUS_FAIL;
  if (urpc_udp_client->fail)
    return URPC_STATUS_TRANSPORT_ERROR;

  iheader = urpc_data_get_header (urpc_udp_client->urpc_data, URPC_DATA_INPUT);
  oheader = urpc_data_get_header (urpc_udp_client->urpc_data, URPC_DATA_OUTPUT);
  recv_size = UINT32_FROM_BE (oheader->size);

  /* Время начала передачи. */
  urpc_timer_start (urpc_udp_client->timer);

  /* Отправка запроса. */
  if (send (urpc_udp_client->socket, (void *) oheader, recv_size, 0) < 0)
    {
      urpc_udp_client->fail = 1;
      return URPC_STATUS_TRANSPORT_ERROR;
    }

  /* Ожидание ответа в течение времени urpc_udp_client->timeout. */
  while (urpc_timer_elapsed (urpc_udp_client->timer) < urpc_udp_client->timeout)
    {

      /* Проверяем приход ответа с интервалом в 100мс. */
      FD_ZERO (&sock_set);
      FD_SET (urpc_udp_client->socket, &sock_set);
      sock_tv.tv_sec = 0;
      sock_tv.tv_usec = 100000;

      if (select ((int) (urpc_udp_client->socket + 1), &sock_set, NULL, NULL, &sock_tv) < 0)
        {
          if (urpc_network_last_error () == URPC_EINTR)
            continue;
          urpc_udp_client->fail = 1;
          return URPC_STATUS_TRANSPORT_ERROR;
        }

      /* Если данных нет - ждём. */
      if (!FD_ISSET (urpc_udp_client->socket, &sock_set))
        continue;

      /* Считываем ответ. */
      recv_size = recv (urpc_udp_client->socket, (void *) iheader, URPC_DEFAULT_BUFFER_SIZE, 0);
      if (recv_size < 0)
        {
          if (urpc_network_last_error () == URPC_EINTR)
            continue;
          urpc_udp_client->fail = 1;
          return URPC_STATUS_TRANSPORT_ERROR;
        }

      /* Проверяем заголовок ответа. */
      if (UINT32_FROM_BE (iheader->size) != recv_size)
        continue;
      if (UINT32_FROM_BE (iheader->magic) != URPC_MAGIC)
        continue;

      urpc_data_set_data_size (urpc_udp_client->urpc_data, URPC_DATA_INPUT, recv_size - URPC_HEADER_SIZE);

      return URPC_STATUS_OK;
    }

  return URPC_STATUS_TIMEOUT;
}

const char *
urpc_udp_client_get_self_address (uRpcUDPClient *urpc_udp_client)
{
  if (urpc_udp_client->urpc_udp_client_type != URPC_UDP_CLIENT_TYPE)
    return NULL;
  if (urpc_udp_client->fail)
    return NULL;

  return urpc_udp_client->self_address;
}

const char *
urpc_udp_client_get_peer_address (uRpcUDPClient *urpc_udp_client)
{
  if (urpc_udp_client->urpc_udp_client_type != URPC_UDP_CLIENT_TYPE)
    return NULL;
  if (urpc_udp_client->fail)
    return NULL;

  return urpc_udp_client->peer_address;
}
