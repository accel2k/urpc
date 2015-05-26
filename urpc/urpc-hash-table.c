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


struct uRpcHashTable {

  uint32_t          type;                   // Тип объекта uRpcHashTable.

  uint32_t          nnodes;                 // Число объектов.
  HashNode        **nodes;                  // Хэш таблица.

  uRpcMemChunk     *chunks;                 // Аллокатор ключей.

  urpc_hash_table_destroy_callback value_destroy_func;

};


uRpcHashTable *urpc_hash_table_create( urpc_hash_table_destroy_callback value_destroy_func )
{

  uRpcHashTable *hash_table;
  int i;

  hash_table = malloc( sizeof( uRpcHashTable ) );
  if( hash_table == NULL ) return NULL;

  hash_table->nodes = malloc( HASH_TABLE_SIZE * sizeof( HashNode* ) );
  if( hash_table->nodes == NULL ) { free( hash_table ); return NULL; }

  hash_table->chunks = urpc_mem_chunk_create( sizeof( HashNode ) );
  if( hash_table->chunks == NULL ) { free( hash_table->nodes ); free( hash_table ); return NULL; }

  hash_table->nnodes = 0;
  for( i = 0; i < HASH_TABLE_SIZE; i++ )
    hash_table->nodes[i] = NULL;

  hash_table->value_destroy_func = value_destroy_func;

  hash_table->type = URPC_HASH_TABLE_TYPE;

  return hash_table;

}


void urpc_hash_table_destroy( uRpcHashTable *hash_table )
{

  HashNode *node;
  uint32_t i;

  if( hash_table->type != URPC_HASH_TABLE_TYPE ) return;

  if( hash_table->value_destroy_func != NULL )
    for( i = 0; i < HASH_TABLE_SIZE; i++ )
      {
      node = hash_table->nodes[i];
      if( node == NULL ) continue;
      while( node != NULL )
        {
        hash_table->value_destroy_func( node->value );
        node = node->next;
        }
      }

  urpc_mem_chunk_destroy( hash_table->chunks );
  free( hash_table->nodes );
  free( hash_table );

}


int urpc_hash_table_insert( uRpcHashTable *hash_table, uint32_t key, void *value )
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
  hash_table->nnodes += 1;

  return 0;

}


int urpc_hash_table_insert_uint32( uRpcHashTable *hash_table, uint32_t key, uint32_t value )
{

  return urpc_hash_table_insert( hash_table, key, (void*)(uintptr_t)value );

}


void *urpc_hash_table_find( uRpcHashTable *hash_table, uint32_t key )
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


uint32_t urpc_hash_table_find_uint32( uRpcHashTable *hash_table, uint32_t key )
{

  return (uintptr_t)urpc_hash_table_find( hash_table, key );

}


void urpc_hash_table_foreach( uRpcHashTable *hash_table, urpc_hash_table_foreach_callback callback, void *user_data )
{

  HashNode *node, *next;
  uint32_t i;

  if( hash_table->type != URPC_HASH_TABLE_TYPE ) return;

  for( i = 0; i < HASH_TABLE_SIZE; i++ )
    {
    node = hash_table->nodes[i];
    if( node == NULL ) continue;
    while( node != NULL )
      {
      next = node->next;
      callback( node->key, node->value, user_data );
      node = next;
      }
    }

}


uint32_t urpc_hash_table_size( uRpcHashTable *hash_table )
{

  if( hash_table->type != URPC_HASH_TABLE_TYPE ) return 0;

  return hash_table->nnodes;

}


int urpc_hash_table_remove( uRpcHashTable *hash_table, uint32_t key )
{

  HashNode *parrent = NULL;
  HashNode *node;

  if( hash_table->type != URPC_HASH_TABLE_TYPE ) return -1;

  node = hash_table->nodes[ key % HASH_TABLE_SIZE ];
  while( node != NULL )
    {
    if( node->key == key )
      {
      if( parrent != NULL ) parrent->next = node->next;
      else hash_table->nodes[ key % HASH_TABLE_SIZE ] = node->next;
      if( hash_table->value_destroy_func != NULL ) hash_table->value_destroy_func( node->value );
      urpc_mem_chunk_free( hash_table->chunks, node );
      hash_table->nnodes -= 1;
      return 0;
      }
    parrent = node;
    node = node->next;
    }

  return 1;

}
