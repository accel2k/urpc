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

/**
 * \file urpc-mem-chunk.h
 *
 * \brief Заголовочный файл библиотеки хранения множества объектов
 * \author Andrei Fadeev (andrei@webcontrol.ru)
 * \date 2015
 * \license GNU General Public License version 3 или более поздняя<br>
 * Коммерческая лицензия - свяжитесь с автором
 *
 * \defgroup uRpcMemChunk uRpcMemChunk - библиотека хранения множества объектов.
 *
 * Библиотека предназначена для хранения большого числа небольших, одинаковых по размеру
 * объектов. Библиотека эффективно работает с числом объектов до 100000. Интерфейс
 * библиотеки содержит следующие функции:
 *
 * - #urpc_mem_chunk_create - создание блока объектов;
 * - #urpc_mem_chunk_destroy - удаление блока объектов;
 * - #urpc_mem_chunk_alloc - выделение памяти под новый объект;
 * - #urpc_mem_chunk_free - освобождение памяти неиспользуемого объекта.
 *
 */

#ifndef __URPC_MEM_CHUNK_H__
#define __URPC_MEM_CHUNK_H__

#include <urpc-exports.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct _uRpcMemChunk uRpcMemChunk;

/**
 *
 * Функция создаёт блок объектов. При создании указывается объекты
 * какого размера будут использоваться.
 *
 * \param chunk_size размер одного объекта.
 *
 * \return Указатель на блок объектов или NULL в случае ошибки.
 *
*/
URPC_EXPORT
uRpcMemChunk          *urpc_mem_chunk_create   (int            chunk_size);

/**
 *
 * Функция освобождает всю память занятую объектами. С этого момента
 * использовать указатели на объекты нельзя.
 *
 * \param mem_chunk указатель на блок объектов.
 *
 * \return Нет.
 *
 */
URPC_EXPORT
void                   urpc_mem_chunk_destroy  (uRpcMemChunk  *mem_chunk);

/**
 *
 * Функция выделяет память под новый объект.
 *
 * \param mem_chunk указатель на блок объектов.
 *
 * \return Указатель на память для нового объекта или NULL в случае ошибки.
 *
 */
URPC_EXPORT
void                  *urpc_mem_chunk_alloc    (uRpcMemChunk  *mem_chunk);

/**
 *
 * Функция освобождает память используемую объектом.
 *
 * \param mem_chunk указатель на блок объектов;
 * \param chunk указатель на память освобождаемого объекта.
 *
 * \return 0 - если память успешно особождена, -1 в случае ошибки.
 *
 */
URPC_EXPORT
int                    urpc_mem_chunk_free     (uRpcMemChunk  *mem_chunk,
                                                void          *chunk);

#ifdef __cplusplus
}
#endif

#endif /* __URPC_MEM_CHUNK_H__ */
