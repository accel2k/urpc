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
#include <string.h>
#include "urpc-common.h"
#include "urpc-timer.h"
#include "urpc-mutex.h"
#include "urpc-thread.h"
#include "urpc-server.h"
#include "urpc-client.h"

#define URPC_TEST_PROC                     URPC_PROC_USER + 1
#define URPC_TEST_PARAM_ARRAY              URPC_PARAM_USER + 1

char *uri = NULL;
double timeout = URPC_DEFAULT_SESSION_TIMEOUT;
unsigned int payload_size = 1024;
unsigned int threads_num = 1;
unsigned int requests_num = 1000;
unsigned int iterations_num = 0;
unsigned int servers_num = 0;
unsigned int run_server = 0;
unsigned int run_clients = 0;
unsigned int dry_run = 0;
unsigned int show_help = 0;

volatile int running_clients = 0;
volatile int start = 0;
volatile int fail = 0;
uRpcMutex lock;

void
help (char *prog_name)
{
  printf ("\nUsage:\n");
  printf ("  %s: [OPTION...] URI\n\n", prog_name);
  printf ("Options:\n");
  printf ("  -w, --timeout     RPC timeout (default: 5.0)\n");
  printf ("  -s, --size        RPC payload size (default: 1024)\n");
  printf ("  -t, --threads     Number of working client threads (default: 1)\n");
  printf ("  -r, --requests    Number of RPC requests per thread (default: 1000)\n");
  printf ("  -i, --iterations  Number of test iterations per threads (default: 1)\n");
  printf ("  -n, --dry-run     Don't perform received data verification\n");
  printf ("  --servers         Number of working server threads (default: same as clients)\n");
  printf ("  --server-only     Run only server (default: server and clients)\n");
  printf ("  --clients-only    Run only clients (default: server and clients)\n");
  printf ("\n\n");
  exit (0);
}

int
urpc_test_proc (uint32_t  session,
                uRpcData *urpc_data,
                void     *proc_data,
                void     *key_data)
{
  uint8_t *array1;
  uint8_t *array2;
  uint32_t array_size;
  unsigned int i;

  array1 = urpc_data_get (urpc_data, URPC_TEST_PARAM_ARRAY, &array_size);
  array2 = urpc_data_set (urpc_data, URPC_TEST_PARAM_ARRAY, NULL, array_size);

  if (!dry_run)
    {
      for (i = 0; i < array_size / 2; i++)
        {
          array2[i] = array1[array_size - 1 - i];
          array2[array_size - 1 - i] = array1[i];
        }
    }

  return 0;
}

void *
urpc_test_client_proc (void *data)
{
  uRpcClient *client;
  uint32_t client_id;

  uRpcData *urpc_data = NULL;

  uRpcTimer *timer;
  double elapsed = 0.0;

  uint8_t *array1;
  uint8_t *array2;
  uint32_t array_size;
  unsigned int i, j;

  client = urpc_client_create (uri, payload_size + 128, timeout);
  if (client == NULL)
    {
      printf ("error creating uRPC client\n");
      fail = 1;
      return NULL;
    }

  if (urpc_client_connect (client) < 0)
    {
      printf ("error connecting uRPC client to server\n");
      fail = 1;
      return NULL;
    }

  timer = urpc_timer_create ();
  if (timer == NULL)
    {
      printf ("error creating client timer\n");
      fail = 1;
      return NULL;
    }

  array1 = malloc (payload_size);
  if (array1 == NULL)
    {
      printf ("error allocating memory buffer\n");
      fail = 1;
      return NULL;
    }

  urpc_mutex_lock (&lock);
  client_id = running_clients += 1;
  printf ("uRPC client %d is connected to server\n", client_id);
  fflush (stdout);
  urpc_mutex_unlock (&lock);

  while (!start);

  urpc_timer_start (timer);

  for (i = 0; i < requests_num; i++)
    {
      for (j = 0; j < payload_size; j++)
        array1[j] = i + j;

      urpc_data = urpc_client_lock (client);
      if (urpc_data == NULL)
        {
          fail = 1;
          break;
        }

      urpc_data_set (urpc_data, URPC_TEST_PARAM_ARRAY, array1, payload_size);

      if (urpc_client_exec (client, URPC_TEST_PROC) != URPC_STATUS_OK)
        {
          fail = 1;
          break;
        }

      array2 = urpc_data_get (urpc_data, URPC_TEST_PARAM_ARRAY, &array_size);
      if (array_size != payload_size)
        {
          fail = 1;
          break;
        }

      if (!dry_run)
        {
          for (j = 0; j < array_size; j++)
            {
              if (array1[j] != array2[array_size - 1 - j])
                fail = 1;
              if (fail)
                break;
            }
        }

      urpc_client_unlock (client);
      urpc_data = NULL;
    }

  elapsed = urpc_timer_elapsed (timer);

  if (urpc_data != NULL)
    urpc_client_unlock (client);

  if (fail)
    printf ("client %d failed, size %d\n", client_id, payload_size);

  printf ("uRPC client %d: %.0lf RPC/s, %.2lf Mb/s\n", client_id, requests_num / elapsed,
          2.0 * (requests_num / elapsed * payload_size) / (1024.0 * 1024.0));

  urpc_timer_destroy (timer);
  free (array1);

  urpc_client_destroy (client);

  urpc_mutex_lock (&lock);
  running_clients -= 1;
  urpc_mutex_unlock (&lock);

  return NULL;
}

int
main (int    argc,
      char **argv)
{

  uRpcServer *server = NULL;
  uRpcThread **clients;
  int local_running_clients;
  unsigned int i;

  /* Разбор командной строки. */
  {
    int i;

    if (argc == 1)
      help (argv[0]);

    for (i = 1; i < argc; i++)
      {
        if (strcmp (argv[i], "--server-only") == 0)
          {
            run_server = 1;
            continue;
          }

        if (strcmp (argv[i], "--clients-only") == 0)
          {
            run_clients = 1;
            continue;
          }

        if ((strcmp (argv[i], "-n") == 0) || strcmp (argv[i], "--dry-run") == 0)
          {
            dry_run = 1;
            continue;
          }

        if ((strcmp (argv[i], "-h") == 0) || strcmp (argv[i], "--help") == 0)
          {
            show_help = 1;
            continue;
          }

        if ((strcmp (argv[i], "-s") == 0) || strcmp (argv[i], "--size") == 0)
          {
            i += 1;
            payload_size = atoi (argv[i]);
            continue;
          }

        if ((strcmp (argv[i], "-t") == 0) || strcmp (argv[i], "--threads") == 0)
          {
            i += 1;
            threads_num = atoi (argv[i]);
            continue;
          }

        if (strcmp (argv[i], "--servers") == 0)
          {
            i += 1;
            servers_num = atoi (argv[i]);
            continue;
          }

        if ((strcmp (argv[i], "-w") == 0) || strcmp (argv[i], "--timeout") == 0)
          {
            i += 1;
            timeout = atof (argv[i]);
            continue;
          }

        if ((strcmp (argv[i], "-r") == 0) || strcmp (argv[i], "--requests") == 0)
          {
            i += 1;
            requests_num = atoi (argv[i]);
            continue;
          }

        if ((strcmp (argv[i], "-i") == 0) || strcmp (argv[i], "--iterations") == 0)
          {
            i += 1;
            iterations_num = atoi (argv[i]);
            continue;
          }

        if (i == argc - 1)
          {
            uri = argv[i];
            break;
          }

        fprintf (stderr, "%s: unknown option '%s' in command line\n", argv[0], argv[i]);
      }

    if (show_help || uri == NULL)
      help (argv[0]);

    if (run_server == 0 && run_clients == 0)
      {
        run_server = 1;
        run_clients = 1;
      }

    if (payload_size < 4)
      payload_size = 4;
    if (payload_size % 2 != 0)
      payload_size -= 1;
    if (iterations_num == 0)
      iterations_num = 1;
    if (servers_num == 0)
      servers_num = threads_num;
  }

  if (urpc_get_type (uri) == URPC_UDP && payload_size > URPC_DEFAULT_DATA_SIZE - 128)
    {
      printf ("uRPC: truncating payload size to %d bytes due to UDP transport.\n",
              URPC_DEFAULT_DATA_SIZE - 128);
      payload_size = URPC_DEFAULT_DATA_SIZE - 128;
    }

  if ((urpc_get_type (uri) == URPC_TCP || urpc_get_type (uri) == URPC_TCP)
      && payload_size > URPC_MAX_DATA_SIZE - 128)
    {
      printf ("uRPC: truncating payload size to %d bytes (maximum allowable).\n", URPC_MAX_DATA_SIZE - 128);
      payload_size = URPC_MAX_DATA_SIZE - 128;
    }

  if (run_server)
    {
      server = urpc_server_create (uri, servers_num, FD_SETSIZE, URPC_DEFAULT_SESSION_TIMEOUT,
                                   payload_size + 128, timeout);
      if (server == NULL)
        {
          printf ("error creating uRPC server\n");
          return -1;
        }

      urpc_server_add_proc (server, URPC_TEST_PROC, urpc_test_proc, NULL);

      if (urpc_server_bind (server) < 0)
        {
          printf ("error starting uRPC server\n");
          return -1;
        }

      printf ("uRPC server started with %d threads\n", servers_num);
      fflush (stdout);
    }

  urpc_mutex_init (&lock);

  if (run_clients)
    {
      clients = malloc (threads_num * sizeof (uRpcThread *));
      if (clients == NULL)
        {
          printf ("error allocating memory for clients\n");
          return -1;
        }

      for (i = 0; i < threads_num; i++)
        {
          clients[i] = urpc_thread_create (urpc_test_client_proc, NULL);
          if (clients[i] == NULL)
            {
              printf ("error starting uRPC client thread\n");
              return -1;
            }
        }

      do
        {
          urpc_mutex_lock (&lock);
          local_running_clients = running_clients;
          urpc_mutex_unlock (&lock);
          urpc_timer_sleep (0.1);
        }
      while (local_running_clients != threads_num && !fail);

      urpc_timer_sleep (2.0);
      if (!fail)
        start = 1;

      do
        {
          urpc_mutex_lock (&lock);
          local_running_clients = running_clients;
          urpc_mutex_unlock (&lock);
          urpc_timer_sleep (0.1);
        }
      while (local_running_clients != 0);

      for (i = 0; i < threads_num; i++)
        urpc_thread_destroy (clients[i]);
      free (clients);
    }

  if (run_server)
    {
      if (!run_clients)
        {
          printf ("Press [Enter] to terminate server...\n");
          fflush (stdout);
          getchar ();
        }
      urpc_server_destroy (server);
    }

  return 0;
}
