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

#include "urpc-semaphore.h"

#include <stdint.h>
#include <windows.h>

#define URPC_SEM_TYPE 0x544D5375

struct _uRpcSem
{
  uint32_t             type;                   /* Тип объекта uRpcSem. */
  HANDLE               sem;                    /* Идентификатор семафора. */
};

uRpcSem *
urpc_sem_create (const char *name,
                 uRpcSemStat stat,
                 int         queue)
{
  uRpcSem *sem = malloc (sizeof (uRpcSem));
  if (sem == NULL)
    return NULL;

  sem->sem = CreateSemaphore (NULL, stat == URPC_SEM_LOCKED ? 0 : queue, queue, name);
  if (sem->sem == NULL)
    {
      free (sem);
      return NULL;
    }

  sem->type = URPC_SEM_TYPE;

  return sem;
}

uRpcSem *
urpc_sem_open (const char *name)
{
  uRpcSem *sem = malloc (sizeof (uRpcSem));
  if (sem == NULL)
    return NULL;

  sem->sem = OpenSemaphore (SEMAPHORE_ALL_ACCESS, FALSE, name);
  if (sem->sem == NULL)
    {
      free (sem);
      return NULL;
    }

  sem->type = URPC_SEM_TYPE;

  return sem;
}

void
urpc_sem_destroy (uRpcSem *sem)
{
  if (sem->type != URPC_SEM_TYPE)
    return;

  CloseHandle (sem->sem);
  free (sem);
}

void
urpc_sem_remove (const char *name)
{
}

void
urpc_sem_lock (uRpcSem *sem)
{
  if (sem->type != URPC_SEM_TYPE)
    return;

  while (WaitForSingleObject (sem->sem, INFINITE) != WAIT_OBJECT_0);
}

int
urpc_sem_trylock (uRpcSem *sem)
{
  if (sem->type != URPC_SEM_TYPE)
    return -1;

  return WaitForSingleObject (sem->sem, 0L) == WAIT_OBJECT_0 ? 0 : -1;
}

int
urpc_sem_timedlock (uRpcSem *sem,
                    double   time)
{
  DWORD wait_time = (DWORD)(1000 * time);
  DWORD wait_stat;

  if (sem->type != URPC_SEM_TYPE)
    return -1;

  wait_stat = WaitForSingleObject (sem->sem, wait_time);

  if (wait_stat == WAIT_OBJECT_0)
    return 0;

  if (wait_stat == WAIT_TIMEOUT)
    return 1;

  return -1;
}

void
urpc_sem_unlock (uRpcSem *sem)
{
  if (sem->type != URPC_SEM_TYPE)
    return;

  while (!ReleaseSemaphore( sem->sem, 1,NULL));
}
