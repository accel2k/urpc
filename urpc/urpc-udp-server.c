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

#include "urpc-udp-server.h"
#include "urpc-common.h"
#include "urpc-network.h"
#include "urpc-timer.h"
#include "urpc-endian.h"

#include <stdlib.h>

#define URPC_UDP_SERVER_TYPE 0x53504455

struct _uRpcUDPServer
{
  uint32_t             urpc_udp_server_type;   /* Тип объекта uRpcUDPServer. */

  SOCKET               socket;                 /* Рабочий сокет. */
  struct sockaddr    **client_addr;            /* Массив структур с адресами клиентов. */
  size_t               client_addr_len;        /* Размер адреса клиента. */

  uRpcData           **urpc_data;              /* Указатель на объекты RPC данных. */
  uint32_t             threads_num;            /* Число рабочих потоков. */
};

uRpcUDPServer *
urpc_udp_server_create (const char *uri,
                        uint32_t    threads_num,
                        double      timeout)
{
  uRpcUDPServer *urpc_udp_server = NULL;
  struct addrinfo *addr = NULL;
  unsigned int i;

  /* Проверка ограничений. */
  if (threads_num > URPC_MAX_THREADS_NUM)
    threads_num = URPC_MAX_THREADS_NUM;

  /* Проверяем тип адреса. */
  if (urpc_get_type (uri) != URPC_UDP)
    return NULL;

  /* Структура объекта. */
  urpc_udp_server = malloc (sizeof (uRpcUDPServer));
  if (urpc_udp_server == NULL)
    return NULL;

  urpc_udp_server->urpc_udp_server_type = URPC_UDP_SERVER_TYPE;
  urpc_udp_server->socket = INVALID_SOCKET;
  urpc_udp_server->client_addr = NULL;
  urpc_udp_server->urpc_data = NULL;
  urpc_udp_server->threads_num = threads_num;

  /* Буферы приёма-передачи. */
  urpc_udp_server->urpc_data = malloc (threads_num * sizeof (uRpcData *));
  if (urpc_udp_server->urpc_data == NULL)
    goto urpc_udp_server_create_fail;
  for (i = 0; i < threads_num; i++)
    urpc_udp_server->urpc_data[i] = NULL;

  for (i = 0; i < threads_num; i++)
    {
      urpc_udp_server->urpc_data[i] = urpc_data_create (URPC_DEFAULT_BUFFER_SIZE, sizeof (uRpcHeader),
                                                        NULL, NULL, 0);
      if (urpc_udp_server->urpc_data[i] == NULL)
        goto urpc_udp_server_create_fail;
    }

  /* Адрес сервера. */
  addr = urpc_get_sockaddr (uri);
  if (addr == NULL)
    goto urpc_udp_server_create_fail;

  /* Рабочий сокет. */
  urpc_udp_server->socket = socket (addr->ai_family, SOCK_DGRAM, addr->ai_protocol);
  if (urpc_udp_server->socket == INVALID_SOCKET)
    goto urpc_udp_server_create_fail;
  urpc_network_set_non_block (urpc_udp_server->socket);
  urpc_network_set_reuse (urpc_udp_server->socket);
  if (bind (urpc_udp_server->socket, addr->ai_addr, (socklen_t) addr->ai_addrlen) < 0)
    goto urpc_udp_server_create_fail;

  /* Структуры для сохранения адресов клиентов. */
  urpc_udp_server->client_addr = malloc (threads_num * sizeof (struct sockaddr *));
  if (urpc_udp_server->client_addr == NULL)
    goto urpc_udp_server_create_fail;
  for (i = 0; i < threads_num; i++)
    urpc_udp_server->client_addr[i] = NULL;

  for (i = 0; i < threads_num; i++)
    {
      urpc_udp_server->client_addr[i] = malloc (addr->ai_addrlen);
      if (urpc_udp_server->client_addr[i] == NULL)
        goto urpc_udp_server_create_fail;
    }

  urpc_udp_server->client_addr_len = addr->ai_addrlen;

  freeaddrinfo (addr);

  return urpc_udp_server;

urpc_udp_server_create_fail:
  urpc_udp_server_destroy (urpc_udp_server);
  if (addr != NULL)
    freeaddrinfo (addr);

  return NULL;
}

void
urpc_udp_server_destroy (uRpcUDPServer *urpc_udp_server)
{
  unsigned int i;

  if (urpc_udp_server->urpc_udp_server_type != URPC_UDP_SERVER_TYPE)
    return;

  /* Закрываем рабочий сокет. */
  if (urpc_udp_server->socket != INVALID_SOCKET)
    closesocket (urpc_udp_server->socket);

  if (urpc_udp_server->client_addr != NULL)
    {
      for (i = 0; i < urpc_udp_server->threads_num; i++)
        {
          if (urpc_udp_server->client_addr[i] != NULL)
            free (urpc_udp_server->client_addr[i]);
        }
      free (urpc_udp_server->client_addr);
    }

  /* Освобождаем память буферов приёма-передачи. */
  if (urpc_udp_server->urpc_data != NULL)
    {
      for (i = 0; i < urpc_udp_server->threads_num; i++)
        {
          if (urpc_udp_server->urpc_data[i] != NULL)
            urpc_data_destroy (urpc_udp_server->urpc_data[i]);
        }
      free (urpc_udp_server->urpc_data);
    }

  free (urpc_udp_server);
}

uRpcData *
urpc_udp_server_recv (uRpcUDPServer *urpc_udp_server,
                      uint32_t       thread_id)
{
  fd_set sock_set;
  struct timeval sock_tv;

  uRpcData *urpc_data;
  uRpcHeader *iheader;
  socklen_t client_addr_len;
  int recv_size;

  if (urpc_udp_server->urpc_udp_server_type != URPC_UDP_SERVER_TYPE)
    return NULL;
  if (thread_id > urpc_udp_server->threads_num - 1)
    return NULL;

  urpc_data = urpc_udp_server->urpc_data[thread_id];
  iheader = urpc_data_get_header (urpc_data, URPC_DATA_INPUT);

  /* Ожидаем запрос в течение 500мс. */
  FD_ZERO (&sock_set);
  FD_SET (urpc_udp_server->socket, &sock_set);
  sock_tv.tv_sec = 0;
  sock_tv.tv_usec = 500000;

  /* Ждём данные. */
  if (select ((int) (urpc_udp_server->socket + 1), &sock_set, NULL, NULL, &sock_tv) < 0)
    return NULL;
  if (!FD_ISSET (urpc_udp_server->socket, &sock_set))
    return NULL;

  /* Считываем данные. */
  client_addr_len = (socklen_t) urpc_udp_server->client_addr_len;
  recv_size = recvfrom (urpc_udp_server->socket, (void *) iheader, URPC_DEFAULT_BUFFER_SIZE, 0,
                        urpc_udp_server->client_addr[thread_id], &client_addr_len);
  if (client_addr_len != urpc_udp_server->client_addr_len)
    return NULL;
  if (recv_size < 0)
    return NULL;

  /* Проверяем заголовок запроса. */
  if (UINT32_FROM_BE (iheader->size) != recv_size)
    return NULL;
  if (UINT32_FROM_BE (iheader->magic) != URPC_MAGIC)
    return NULL;

  urpc_data_set_data_size (urpc_data, URPC_DATA_INPUT, recv_size - URPC_HEADER_SIZE);

  return urpc_data;
}

int
urpc_udp_server_send (uRpcUDPServer *urpc_udp_server,
                      uint32_t       thread_id)
{
  uRpcData *urpc_data;
  uRpcHeader *oheader;
  int send_size;
  int sended;

  if (urpc_udp_server->urpc_udp_server_type != URPC_UDP_SERVER_TYPE)
    return -1;
  if (thread_id > urpc_udp_server->threads_num - 1)
    return -1;

  /* Отправляемые данные. */
  urpc_data = urpc_udp_server->urpc_data[thread_id];
  oheader = urpc_data_get_header (urpc_data, URPC_DATA_OUTPUT);
  send_size = UINT32_FROM_BE (oheader->size);

  /* Отправка ответа. */
  sended = sendto (urpc_udp_server->socket, (void *) oheader, send_size, 0, 
                   urpc_udp_server->client_addr[thread_id],
                   (socklen_t)urpc_udp_server->client_addr_len);
  if (sended < 0)
    return -1;

  return 0;
}
