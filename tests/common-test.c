/*
 * uRPC - rpc (remote procedure call) library.
 *
 * Copyright 2015 Andrei Fadeev (andrei@webcontrol.ru)
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
#include <stdlib.h>
#include "urpc-common.h"

#define ERROR_CODE -1

int
main (int    argc,
      char **argv)
{
  int i = 0;
  char *uri[] =
    {
      "tcp://[::1234:7890]:12345/",
      "tcp://127.0.0.1:12345/",
      "tcp://localhost:12345/",
      "tcp://www.ya.ru:12345/",
      "tcp://*:12345/",
      "udp://[::1234:7890]:12345/",
      "udp://127.0.0.1:12345/",
      "udp://localhost:12345/",
      "udp://www.ya.ru:12345/",
      "udp://*:12345/",
      NULL
    };

  urpc_network_init ();

  while (uri[i] != NULL)
    {
      struct addrinfo *addr;
      addr = urpc_get_sockaddr (uri[i]);
      if (addr == NULL)
        {
        printf ("error resolving %s\n", uri[i]);
        exit (ERROR_CODE);
        }
      freeaddrinfo (addr);
      i += 1;
    }

  printf ("All done\n");

  urpc_network_close ();

  return 0;
}
