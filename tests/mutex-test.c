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
#include "urpc-mutex.h"
#include "urpc-thread.h"

#define ERROR -1

volatile int start = 0;

uRpcMutex mutex;

int counts = 2500000;
int sum = 0;

void *
thread_func (void *data)
{
  int i;

  while (start == 0);

  for (i = 0; i < counts; i++)
    {
      urpc_mutex_lock (&mutex);
      sum++;
      urpc_mutex_unlock (&mutex);
    }

  return NULL;
}


int
main (int argc, char **argv)
{
  uRpcThread *thread1;
  uRpcThread *thread2;

  urpc_mutex_init (&mutex);
  thread1 = urpc_thread_create (thread_func, NULL);
  thread2 = urpc_thread_create (thread_func, NULL);

  start = 1;

  urpc_thread_destroy (thread1);
  urpc_thread_destroy (thread2);
  urpc_mutex_clear (&mutex);

  if ( sum != 2 * counts )
    {
      printf ("mutex error in threads");
      exit (ERROR);
    }

  printf ("All done");

  return 0;
}
