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
#include "urpc-shm.h"
#include "urpc-semaphore.h"

#define ERROR_CODE -1

#define SHM_NAME "urpc-server-test"
#define SEM_START_NAME "urpc-server-test-start"
#define SEM_STOP_NAME "urpc-server-test-stop"

int
main (int    argc,
      char **argv)
{
  uRpcShm *server_shm;
  uRpcSem *server_start_sem;
  uRpcSem *server_stop_sem;

  double *io;

  server_shm = urpc_shm_open (SHM_NAME, 2 * sizeof (double));
  server_start_sem = urpc_sem_open (SEM_START_NAME);
  server_stop_sem = urpc_sem_open (SEM_STOP_NAME);

  io = urpc_shm_map (server_shm);

  io[0] = 1.23456;
  io[1] = 0.0;
  urpc_sem_unlock (server_start_sem);
  urpc_sem_lock (server_stop_sem);

  if (io[1] != 1.23456 * 1.23456)
    {
      printf ("communication error io[0] = %lf, io[1] = %lf, local = %lf\n", io[0], io[1], 1.23456 * 1.23456);
      exit (ERROR_CODE);
    }

  printf ("All done\n");

  return 0;
}
