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


#define URPC_HASH_TABLE_TYPE 0x54544875
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

  uRpcHashTable *hash_table;
  int i;

  hash_table = malloc( HASH_TABLE_SIZE );
  if( hash_table == NULL ) return NULL;

  hash_table->nodes = malloc( HASH_TABLE_SIZE * sizeof( HashNode* ) );
  if( hash_table->nodes == NULL ) { free( hash_table ); return NULL; }

  hash_table->chunks = urpc_mem_chunk_create( sizeof( HashNode ) );
  if( hash_table->chunks == NULL ) { free( hash_table->nodes ); free( hash_table ); return NULL; }

  hash_table->nnodes = 0;
  for( i = 0; i < HASH_TABLE_SIZE; i++ )
    hash_table->nodes[i] = NULL;

  hash_table->type = URPC_HASH_TABLE_TYPE;

  return hash_table;

}


URPC_EXPORT void urpc_hash_table_destroy( uRpcHashTable *hash_table )
{

  if( hash_table->type != URPC_HASH_TABLE_TYPE ) return;

  urpc_mem_chunk_destroy( hash_table->chunks );
  free( hash_table->nodes );
  free( hash_table );

}


URPC_EXPORT int urpc_hash_table_insert( uRpcHashTable *hash_table, uint32_t key, void *value )
{

  HashNode *parrent = NULL;
  HashNode *node;

  if( hash_table->type != URPC_HASH_TABLE_TYPE ) return -1;

  node = hash_table->nodes[ key % HASH_TABLE_SIZE ];
  while( node != NULL )
    {
    if( node->key == key ) return 1;
    parrent = node;
    node = node->next;
    }

  node = urpc_mem_chunk_alloc( hash_table->chunks );
  if( node == NULL ) return -1;
  node->key = key;
  node->value = value;
  node->next = NULL;

  if( parrent == NULL ) hash_table->nodes[ key % HASH_TABLE_SIZE ] = node;
  else parrent->next = node;

  return 0;

}


URPC_EXPORT void *urpc_hash_table_find( uRpcHashTable *hash_table, uint32_t key )
{

  HashNode *node;

  if( hash_table->type != URPC_HASH_TABLE_TYPE ) return NULL;

  node = hash_table->nodes[ key % HASH_TABLE_SIZE ];
  while( node != NULL )
    {
    if( node->key == key ) return node->value;
    node = node->next;
    }

  return NULL;

}


URPC_EXPORT int urpc_hash_table_remove( uRpcHashTable *hash_table, uint32_t key )
{

  HashNode *parrent = NULL;
  HashNode *node;

  if( hash_table->type != URPC_HASH_TABLE_TYPE ) return -1;

  node = hash_table->nodes[ key % HASH_TABLE_SIZE ];
  while( node != NULL )
    {
    if( node->key == key )
      {
      if( parrent != NULL ) parrent = node->next;
      else hash_table->nodes[ key % HASH_TABLE_SIZE ] = node->next;
      urpc_mem_chunk_free( hash_table->chunks, node );
      return 0;
      }
    parrent = node;
    node = node->next;
    }

  return 1;

}
