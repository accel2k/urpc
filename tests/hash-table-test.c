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

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "urpc-hash-table.h"


int main( int argc, char **argv )
{

  uRpcHashTable *uhash;

  uint32_t key;
  int i;

  uhash = urpc_hash_table_create( free );

  for( key = 0; key < 100000; key++ )
    {
    uint32_t *value = malloc( sizeof( uint32_t ) );
    *value = key;
    if( urpc_hash_table_insert( uhash, key, value ) != 0 )
      printf( "error inserting key %d\n", key );
    }

  for( key = 0; key < 100000; key += 10000 )
    if( urpc_hash_table_insert( uhash, key, (void*)key ) <= 0 )
      printf( "error inserting duplicated key %d\n", key );

  for( i = 0; i < 10; i++ )
    for( key = 0; key < 100000; key++ )
      {
      uint32_t *value = urpc_hash_table_find( uhash, key );
      if( *value != key ) printf( "error finding key %d\n", key );
      }

  for( key = 0; key < 100000; key++ )
    if( urpc_hash_table_remove( uhash, key ) != 0 )
      printf( "error removing key %d\n", key );

  urpc_hash_table_destroy( uhash );

  return 0;

}
