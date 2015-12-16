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
#include <stdint.h>
#include <stdlib.h>
#include "urpc-hash-table.h"

#define ERROR -1

void
remove_callback (uint32_t key,
                 void    *value,
                 void    *data)
{
  if (key % 10000 == 0)
    {
      if (urpc_hash_table_remove (data, key) != 0)
        {
          printf ("error removing key %d\n", key);
          exit (ERROR);
        }
    }
}

int
main (int    argc,
      char **argv)
{

  uRpcHashTable *uhash;

  uint32_t key;
  int i;

  uhash = urpc_hash_table_create (free);

  for (key = 0; key < 100000; key++)
    {
      uint32_t *value = malloc (sizeof (uint32_t));
      *value = key;
      if (urpc_hash_table_insert (uhash, key, value) != 0)
        {
          printf ("error inserting key %d\n", key);
          exit (ERROR);
        }
    }

  for (key = 0; key < 100000; key += 10000)
    {
      if (urpc_hash_table_insert_uint32 (uhash, key, key) <= 0)
        {
          printf ("error inserting duplicated key %d\n", key);
          exit (ERROR);
        }
    }

  urpc_hash_table_foreach (uhash, remove_callback, uhash);

  for (i = 0; i < 10; i++)
    {
      for (key = 0; key < 100000; key++)
        {
          uint32_t *value = urpc_hash_table_find (uhash, key);
          if ((value == NULL || *value != key) && (key % 10000 != 0))
            {
              printf ("error finding key %d\n", key);
              exit (ERROR);
            }
          else if (value != NULL && key % 10000 == 0)
            {
              printf ("error finding key %d\n", key);
              exit (ERROR);
            }
        }
    }

  for (key = 0; key < 100000; key++)
    {
      if (urpc_hash_table_remove (uhash, key) != 0)
        {
          if (key % 10000 != 0)
            {
              printf ("error removing key %d\n", key);
              exit (ERROR);
            }
        }
    }

  urpc_hash_table_destroy (uhash);

  printf ("All done\n");

  return 0;
}
