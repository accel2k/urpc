/*
 * uRpc - rpc (remote procedure call) library.
 *
 * Copyright 2009, 2010, 2011, 2015 Andrei Fadeev
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

#include "urpc-network.h"


int urpc_network_init( void )
{

  return 0;

}


void urpc_network_close( void )
{

}


int urpc_network_last_error( void )
{

  return errno;

}


const char* urpc_network_last_error_str( void )
{

  return strerror( errno );

}
