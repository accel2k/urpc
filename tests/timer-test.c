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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "urpc-timer.h"

#define ERROR -1

#if defined( _WIN32 )
#include <Windows.h>
#endif

int
main (int argc, char **argv)
{
  double delta;
  double delay;
  double elapsed;
  uRpcTimer *timer;
  int i;

#if defined( _WIN32 )
  timeBeginPeriod (1);
#endif

  timer = urpc_timer_create ();

  for (i = 0; i < 10; i++)
    {
    delay = 0.025 + 0.1 * i;
    urpc_timer_start (timer);
    urpc_timer_sleep (delay);
    elapsed = urpc_timer_elapsed (timer);
    delta = delay / elapsed;
    if (delta < 0.95 || delta > 1.05)
      {
        printf ("sleep error, requested %.3lfms, real %.3lfms\n", 1000.0 * delay, 1000.0 * elapsed);
        exit (ERROR);
      }
    }

  printf ("All done\n");

  return 0;
}
