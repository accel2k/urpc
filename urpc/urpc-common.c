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

#include "urpc-common.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifndef AI_ADDRCONFIG
#define AI_ADDRCONFIG 0
#endif

#if defined _MSVC_COMPILER
#define snprintf sprintf_s
#endif

uRpcType
urpc_get_type (const char *uri)
{
  uRpcType urpc_type = URPC_UNKNOWN;
  char uri_prefix[6];
  int i;

  if (strlen (uri) < sizeof ("ttt://*"))
    return URPC_UNKNOWN;
  for (i = 0; i < sizeof (uri_prefix); i++)
    uri_prefix[i] = tolower (uri[i]);

  if (memcmp (uri_prefix, "udp://", 6) == 0)
    urpc_type = URPC_UDP;
  if (memcmp (uri_prefix, "tcp://", 6) == 0)
    urpc_type = URPC_TCP;
  if (memcmp (uri_prefix, "shm://", 6) == 0)
    urpc_type = URPC_SHM;

  return urpc_type;
}

struct addrinfo *
urpc_get_sockaddr (const char *uri)
{
  uRpcType urpc_type = urpc_get_type (uri);

  char *delim = NULL;
  int offset = 0;

  char host[MAX_HOST_LEN + 1];
  char port[MAX_PORT_LEN + 1];

  struct addrinfo addr_hint;
  struct addrinfo *addr;
  int any_address = 0;

  char *host_end;
  size_t host_len;
  int gai_ret;

  if (urpc_type == URPC_UNKNOWN || urpc_type == URPC_SHM)
    return NULL;

  /* Для IPV6 разделитель - ']:', для IPV4 и имени - ':'. */
  if (uri[6] == '[')
    {
      delim = "]:";
      offset = 2;
    }
  else
    {
      delim = ":";
      offset = 1;
    }

  /* Окончание адреса или имени. */
  host_end = strstr (uri + 7, delim);
  if (host_end == NULL)
    return NULL;

  /* Копирование адреса или имени в host. */
  host_len = host_end - uri - (4 + offset);
  if (host_len > MAX_HOST_LEN)
    return NULL;
  memcpy (host, uri + 5 + offset, host_len - 1);
  host[host_len - 1] = 0;

  /* Копирование номера порта в port. */
  snprintf (port, MAX_PORT_LEN + 1, "%d", atoi (host_end + offset));

  /* Если имя хоста = '*', настраиваем приём по любому адресу. */
  if (host[0] == '*' && host[1] == 0)
    any_address = 1;

  /* Перевод uri в сетевой адрес. */
  memset (&addr_hint, 0, sizeof (addr_hint));
  addr_hint.ai_family = AF_UNSPEC;
  if (urpc_type == URPC_TCP)
    {
      addr_hint.ai_socktype = SOCK_STREAM;
      addr_hint.ai_protocol = IPPROTO_TCP;
    }
  else
    {
      addr_hint.ai_socktype = SOCK_DGRAM;
      addr_hint.ai_protocol = IPPROTO_UDP;
    }
  addr_hint.ai_flags = AI_ADDRCONFIG;
  if (any_address)
    addr_hint.ai_flags |= AI_PASSIVE;

  if ((gai_ret = getaddrinfo (any_address ? NULL : host, port, &addr_hint, &addr)) != 0)
    {
      fprintf (stderr, "urpc_get_sockaddr: getaddrinfo('%s'): %s\n", uri, gai_strerror (gai_ret));
      addr = NULL;
    }

  return addr;
}
