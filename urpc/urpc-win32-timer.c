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

#include "urpc-timer.h"
#include "urpc-network.h"

#include <stdint.h>

#define URPC_TIMER_TYPE 0x54525475

struct _uRpcTimer
{
  uint32_t             type;                   /* Тип объекта uRpcTimer. */
  LARGE_INTEGER        pfreq;                  /* Частота обновления PerformanceCounter'а. */
  LARGE_INTEGER        start;                  /* Начальный момент времени. */
};

uRpcTimer *
urpc_timer_create (void)
{
  uRpcTimer *timer = malloc (sizeof (uRpcTimer));

  if (timer == NULL)
    return NULL;

  QueryPerformanceFrequency (&timer->pfreq);
  QueryPerformanceCounter (&timer->start);
  timer->type = URPC_TIMER_TYPE;

  return timer;
}

void
urpc_timer_destroy (uRpcTimer *timer)
{
  if (timer->type != URPC_TIMER_TYPE)
    return;

  free (timer);
}

void
urpc_timer_start (uRpcTimer *timer)
{
  if (timer->type != URPC_TIMER_TYPE)
    return;

  QueryPerformanceCounter (&timer->start);
}

double
urpc_timer_elapsed (uRpcTimer *timer)
{
  LARGE_INTEGER finish;
  double elapsed;

  if (timer->type != URPC_TIMER_TYPE)
    return -1.0;

  QueryPerformanceCounter (&finish);
  finish.QuadPart = finish.QuadPart - timer->start.QuadPart;
  finish.QuadPart *= 1000000;
  finish.QuadPart /= timer->pfreq.QuadPart;
  elapsed = (double)finish.QuadPart;
  elapsed /= 1000000.0;

  return elapsed;
}

void
urpc_timer_sleep (double time)
{
  Sleep ((DWORD)(1000 * time));
}
