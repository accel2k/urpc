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

#include "urpc-hash-table.h"
#include "urpc-mem-chunk.h"

#include <stdlib.h>


#define URPC_HASH_TABLE_TYPE 0x54544855
#define HASH_TABLE_SIZE      251


typedef struct HashNode HashNode;
struct HashNode {

  uint32_t          key;                    // Значение ключа.
  void             *value;                  // Указатель на данные.
  HashNode         *next;                   // Указатель на следующий узел.

};


typedef struct uRpcHashTable {

  uint32_t          type;                   // Тип объекта uRpcHashTable.

  int               nnodes;                 // Число объектов.
  HashNode        **nodes;                  // Хэш таблица.

  uRpcMemChunk     *chunks;

} uRpcHashTable;


URPC_EXPORT uRpcHashTable *urpc_hash_table_create( void )
{

  uRpcHashTable *uhash;
  int i;

  uhash = malloc( HASH_TABLE_SIZE );
  if( uhash == NULL ) return NULL;

  uhash->nodes = malloc( HASH_TABLE_SIZE * sizeof( HashNode* ) );
  if( uhash->nodes == NULL ) { free( uhash ); return NULL; }

  uhash->chunks = urpc_mem_chunk_create( sizeof( HashNode ) );
  if( uhash->chunks == NULL ) { free( uhash->nodes ); free( uhash ); return NULL; }

  uhash->nnodes = 0;
  for( i = 0; i < HASH_TABLE_SIZE; i++ )
    uhash->nodes[i] = NULL;

  uhash->type = URPC_HASH_TABLE_TYPE;

  return uhash;

}


URPC_EXPORT void urpc_hash_table_destroy( uRpcHashTable *uhash )
{

  if( uhash->type != URPC_HASH_TABLE_TYPE ) return;

  urpc_mem_chunk_destroy( uhash->chunks );
  free( uhash->nodes );
  free( uhash );

}


URPC_EXPORT int urpc_hash_table_insert( uRpcHashTable *uhash, uint32_t key, void *value )
{

  HashNode *parrent = NULL;
  HashNode *node;

  if( uhash->type != URPC_HASH_TABLE_TYPE ) return -1;

  node = uhash->nodes[ key % HASH_TABLE_SIZE ];
  while( node != NULL )
    {
    if( node->key == key ) return 1;
    parrent = node;
    node = node->next;
    }

  node = urpc_mem_chunk_alloc( uhash->chunks );
  if( node == NULL ) return -1;
  node->key = key;
  node->value = value;
  node->next = NULL;

  if( parrent == NULL ) uhash->nodes[ key % HASH_TABLE_SIZE ] = node;
  else parrent->next = node;

  return 0;

}


URPC_EXPORT void *urpc_hash_table_find( uRpcHashTable *uhash, uint32_t key )
{

  HashNode *node;

  if( uhash->type != URPC_HASH_TABLE_TYPE ) return NULL;

  node = uhash->nodes[ key % HASH_TABLE_SIZE ];
  while( node != NULL )
    {
    if( node->key == key ) return node->value;
    node = node->next;
    }

  return NULL;

}


URPC_EXPORT int urpc_hash_table_remove( uRpcHashTable *uhash, uint32_t key )
{

  HashNode *parrent = NULL;
  HashNode *node;

  if( uhash->type != URPC_HASH_TABLE_TYPE ) return -1;

  node = uhash->nodes[ key % HASH_TABLE_SIZE ];
  while( node != NULL )
    {
    if( node->key == key )
      {
      if( parrent != NULL ) parrent = node->next;
      else uhash->nodes[ key % HASH_TABLE_SIZE ] = node->next;
      urpc_mem_chunk_free( uhash->chunks, node );
      return 0;
      }
    parrent = node;
    node = node->next;
    }

  return 1;

}
