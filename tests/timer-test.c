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
#include "urpc-timer.h"

#if defined( _WIN32 )
#include <Windows.h>
#endif


int main( int argc, char **argv )
{

  double elapsed;
  uRpcTimer *timer = urpc_timer_create();

#if defined( _WIN32 )
  timeBeginPeriod( 1 );
#endif

  urpc_timer_sleep( 0.025 );
  elapsed = urpc_timer_elapsed( timer );
  printf( "sleep for 25ms - waked up after %.3lfms\n", 1000.0 * elapsed );

  urpc_timer_start( timer );
  urpc_timer_sleep( 0.125 );
  elapsed = urpc_timer_elapsed( timer );
  printf( "sleep for 125ms - waked up after %.3lfms\n", 1000.0 * elapsed );

  urpc_timer_start( timer );
  urpc_timer_sleep( 0.250 );
  elapsed = urpc_timer_elapsed( timer );
  printf( "sleep for 250ms - waked up after %.3lfms\n", 1000.0 * elapsed );

  urpc_timer_start( timer );
  urpc_timer_sleep( 1.250 );
  elapsed = urpc_timer_elapsed( timer );
  printf( "sleep for 1250ms - waked up after %.3lfms\n", 1000.0 * elapsed );

  return 0;

}
