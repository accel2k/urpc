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

#include "urpc-mem-chunk.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define URPC_MEM_CHUNK_TYPE 0x54434D75

#define DATA_ALIGN          (2 * sizeof (void*))
#define PAGE_SIZE_ALIGN     4096

typedef struct _ChunkPage ChunkPage;

struct _ChunkPage
{
  ChunkPage           *next;                   /* Указатель на следующую страницу. */
  uint32_t             free;                   /* Число свободных слотов для объектов. */
};

struct _uRpcMemChunk
{
  uint32_t             type;                   /* Тип объекта uRpcmemChunk. */

  int                  chunk_size;             /* Размер объектов для которых выделяется память. */
  int                  nchunks_in_page;        /* Число объектов в странице. */

  void                *pages;                  /* Страницы памяти объектов. */
  int                  page_size;              /* Размер страницы. */
  int                  offset;                 /* Смещение до данных от начала страницы. */
};

uRpcMemChunk *
urpc_mem_chunk_create (int chunk_size)
{
  uRpcMemChunk *mem_chunk;
  ChunkPage *chunk_page;

  int page_size;

  mem_chunk = malloc (sizeof (uRpcMemChunk));
  if (mem_chunk == NULL)
    return NULL;

  /* Смещение до данных в каждой странице. Занято заголовком страницу ChunkPage.
     Смещение кратно DATA_ALIGN. */
  mem_chunk->offset = DATA_ALIGN * ((sizeof (ChunkPage) / DATA_ALIGN) +
                      (sizeof (ChunkPage) % DATA_ALIGN ? 1 : 0));

  /* Размер одного блока данных включая служебную информацию: признак занятости и номер страницы.
     Служебная информация хранится в первых 8 байтах блока, затем идут данные пользователя.
     Размер кратен DATA_ALIGN. */
  chunk_size += 2 * sizeof (uint32_t);
  chunk_size = DATA_ALIGN * ((chunk_size / DATA_ALIGN) +
               (chunk_size % DATA_ALIGN ? 1 : 0));
  mem_chunk->chunk_size = chunk_size;

  /* Размер одной страницы. Расчитывается с прицелом на хранение примерно 1024 блоков данных.
     Размер кратен PAGE_SIZE_ALIGN. */
  page_size = (1024 * chunk_size) + mem_chunk->offset;
  page_size = PAGE_SIZE_ALIGN * ((page_size / PAGE_SIZE_ALIGN) +
              ((page_size % PAGE_SIZE_ALIGN > PAGE_SIZE_ALIGN / 2) ? 1 : 0));
  mem_chunk->page_size = page_size;

  /* Число блоков хранящихся в одной странице. */
  mem_chunk->nchunks_in_page = (page_size - mem_chunk->offset) / chunk_size;

  /* Память под начальную страницу выделяется при создании объекта. */
  mem_chunk->pages = malloc (page_size);
  if (mem_chunk->pages == NULL)
    {
      free (mem_chunk);
      return NULL;
    }
  memset (mem_chunk->pages, 0, page_size);

  /* Заголовок страницы. */
  chunk_page = mem_chunk->pages;
  chunk_page->next = NULL;
  chunk_page->free = mem_chunk->nchunks_in_page;

  mem_chunk->type = URPC_MEM_CHUNK_TYPE;

  return mem_chunk;
}

void
urpc_mem_chunk_destroy (uRpcMemChunk *mem_chunk)
{
  ChunkPage *parrent_page;
  ChunkPage *next_page;

  if (mem_chunk->type != URPC_MEM_CHUNK_TYPE)
    return;

  /* По цепочке освобождаем все страницы. */
  parrent_page = mem_chunk->pages;
  while (parrent_page != NULL)
    {
      next_page = parrent_page->next;
      free (parrent_page);
      parrent_page = next_page;
    }

  free (mem_chunk);
}

void *
urpc_mem_chunk_alloc (uRpcMemChunk *mem_chunk)
{
  ChunkPage *chunk_page = mem_chunk->pages;

  uint32_t page = 0;
  uint32_t *data;

  int i;

  if (mem_chunk->type != URPC_MEM_CHUNK_TYPE)
    return NULL;

  /* По цепочке ищем страницу в которой есть свободный блок. */
  while (chunk_page->free == 0)
    {
      /* Если такой страницы не найдено, выделяем память под новую страницу. */
      if (chunk_page->next == NULL)
        {
          chunk_page->next = malloc (mem_chunk->page_size);
          chunk_page = chunk_page->next;
          memset (chunk_page, 0, mem_chunk->page_size);
          chunk_page->next = NULL;
          chunk_page->free = mem_chunk->nchunks_in_page;
          page += 1;
          break;
        }

      chunk_page = chunk_page->next;
      page += 1;
    }

  /* Находим свободный блок в странице. */
  for (i = 0; i < mem_chunk->nchunks_in_page; i++)
    {
      data = (uint32_t *) ((uint8_t *) chunk_page + mem_chunk->offset + i * mem_chunk->chunk_size);
      if (*data == 0)
        break;
    }

  /* Просмотрели все блоки, но не нашли свободный - ошибка в целостности данных. */
  if (i >= mem_chunk->nchunks_in_page)
    return NULL;

  /* Уменьшаем число занятых блоков в странице, помечаем блок как занятый. */
  chunk_page->free -= 1;
  *data = 1;
  data += 1;
  *data = page;
  data += 1;

  return data;
}

int
urpc_mem_chunk_free (uRpcMemChunk *mem_chunk,
                     void         *chunk)
{
  ChunkPage *chunk_page = mem_chunk->pages;

  uint32_t *data = chunk;
  uint32_t *page = data - 1;
  uint32_t *used = data - 2;

  unsigned int i;

  if (mem_chunk->type != URPC_MEM_CHUNK_TYPE)
    return -1;

  /* Находим страницу в которой расположен блок данных. */
  for (i = 0; i < *page; i++)
    {
      if (chunk_page->next == NULL)
        return -1;
      chunk_page = chunk_page->next;
    }

  /* Увеличиваем счётчик свободных блоков и помечаем блок как свободный. */
  chunk_page->free += 1;
  *used = 0;

  return 0;
}
