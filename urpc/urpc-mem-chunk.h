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

/*!
 * \file urpc-mem-chunk.h
 *
 * \author Andrei Fadeev
 * \date 1.04.2015
 * \brief Заголовочный файл библиотеки хранения множества объектов.
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

#ifndef _urpc_mem_chunk_h
#define _urpc_mem_chunk_h

#include <urpc-exports.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct uRpcMemChunk uRpcMemChunk;


/*! Создание блока объектов.
 *
 * При создании указывается объекты какого размера будут использоваться.
 *
 * \param chunk_size размер одного объекта.
 *
 * \return Указатель на блок объектов или NULL в случае ошибки.
 *
*/
URPC_EXPORT uRpcMemChunk *urpc_mem_chunk_create( int chunk_size );


/*! Удаление блока объектов.
 *
 * Функция освобождает всю память занятую объектами. С этого момента
 * использовать указатели на объекты нельзя.
 *
 * \param umem_chunk указатель на блок объектов.
 *
 * \return Нет.
 *
*/
void urpc_mem_chunk_destroy( uRpcMemChunk *umem_chunk );


/*! Выделение памяти под новый объект.
 *
 * \param umem_chunk указатель на блок объектов.
 *
 * \return Указатель на память для нового объекта или NULL в случае ошибки.
 *
*/
void *urpc_mem_chunk_alloc( uRpcMemChunk *umem_chunk );


/*! Освобождение памяти используемой объектом.
 *
 * \param umem_chunk указатель на блок объектов;
 * \param chunk указатель на память освобождаемого объекта.
 *
 * \return 0 - если память успешно особождена, -1 в случае ошибки.
 *
*/
int urpc_mem_chunk_free( uRpcMemChunk *umem_chunk, void *chunk );


#ifdef __cplusplus
} // extern "C"
#endif

#endif // _urpc_mem_chunk_h
