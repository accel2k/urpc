/*
 * uRpc - rpc (remote procedure call) library.
 *
 * Copyright 2015 Andrei Fadeev
 *
 * This file is part of uRPC.
 *
 * uRPC is free software: you can redistribute it and/or modify
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
*/

#include "urpc-timer.h"

#include <time.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>


#define URPC_TIMER_TYPE 0x54525475


struct uRpcTimer {

  uint32_t          type;                   // Тип объекта uRpcTimer.
  struct timespec   start;                  // Начальный момент времени.

};


uRpcTimer *urpc_timer_create( void )
{

  uRpcTimer *timer = malloc( sizeof( uRpcTimer ) );

  if( timer == NULL ) return NULL;

  clock_gettime( CLOCK_MONOTONIC, &timer->start );
  timer->type = URPC_TIMER_TYPE;

  return timer;

}


void urpc_timer_destroy( uRpcTimer *timer )
{

  if( timer->type != URPC_TIMER_TYPE ) return;

  free( timer );

}


void urpc_timer_start( uRpcTimer *timer )
{

  if( timer->type != URPC_TIMER_TYPE ) return;

  clock_gettime( CLOCK_MONOTONIC, &timer->start );

}


double urpc_timer_elapsed( uRpcTimer *timer )
{

  struct timespec stop;
  struct timespec start;

  int sec_diff;
  double elapsed;

  if( timer->type != URPC_TIMER_TYPE ) return -1.0;

  start = timer->start;
  clock_gettime( CLOCK_MONOTONIC, &stop );

  sec_diff = stop.tv_sec - start.tv_sec;
  if( sec_diff == 0 ) elapsed = ( stop.tv_nsec - start.tv_nsec ) / 1000000000.0;
  else
    {
    elapsed = sec_diff - 1;
    elapsed += ( 1000000000 - start.tv_nsec ) / 1000000000.0;
    elapsed += stop.tv_nsec / 1000000000.0;
    }

  return elapsed;

}


void urpc_timer_sleep( double time )
{

  usleep( 1000000.0 * time );

}
