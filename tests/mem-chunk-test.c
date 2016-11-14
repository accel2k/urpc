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
#include <stdlib.h>
#include <stdint.h>
#include "urpc-mem-chunk.h"

#define ERROR_CODE -1

#define N_DATA 250000

typedef struct
{
  uint32_t id;
  uint32_t data[16];
} TestData;

int
main (int    argc,
      char **argv)
{

  TestData **data;
  unsigned int i, j;

  uRpcMemChunk *umem_chunk = urpc_mem_chunk_create (sizeof (TestData) + 1);
  data = malloc (N_DATA * sizeof (TestData *));

  for (i = 0; i < N_DATA; i++)
    {
      data[i] = urpc_mem_chunk_alloc (umem_chunk);
      data[i]->id = i;
      for (j = 0; j < 16; j++)
        data[i]->data[j] = i * j;
    }

  for (i = 0; i < N_DATA; i++)
    {
      if (data[i]->id != i)
        {
          printf ("error in id %u != %u\n", data[i]->id, i);
          exit (ERROR_CODE);
        }
      for (j = 0; j < 16; j++)
        {
          if (data[i]->data[j] != i * j)
            {
              printf ("error in data[%u] %u != %u\n", j, data[i]->data[j], i * j);
              exit (ERROR_CODE);
            }
        }
    }

  for (i = 0; i < N_DATA; i += 10)
    urpc_mem_chunk_free (umem_chunk, data[i]);

  for (i = 0; i < N_DATA; i += 10)
    {
      data[i] = urpc_mem_chunk_alloc (umem_chunk);
      if (data[i]->id != i)
        {
          printf ("realloc error in id %u != %u\n", data[i]->id, i);
          exit (ERROR_CODE);
        }
      for (j = 0; j < 16; j++)
        {
          if (data[i]->data[j] != i * j)
            {
              printf ("realloc error in data[%u] %u != %u\n", j, data[i]->data[j], i * j);
              exit (ERROR_CODE);
            }
        }
    }

  urpc_mem_chunk_destroy (umem_chunk);

  printf ("All done\n");

  return 0;
}
