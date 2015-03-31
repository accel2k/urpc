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
 * \file urpc-hash-table.h
 *
 * \author Andrei Fadeev
 * \date 31.03.2015
 * \brief Заголовочный файл библиотеки работы с хэш таблицей.
 *
 * \defgroup uRpcHashTable uRpcHashTable - библиотека работы с хэш таблицей.
 *
 * Хэш таблица предстваляет собой упрощённый вариант с размером равным 251 и предназначена
 * для хранения указателей на объекты общим числом примерно до 1000 элементов. Большее число
 * элементов может размещаться в таблице, но при этом возрастают накладные расходы на добавление
 * и поиск.
 *
 * Хэш таблица сохраняет указатели на данные ассоциированные с ключами и по запросу возвращает эти
 * указатели. Удаление ключа из таблицы или удаление всей таблицы целиком не приводит к каким-либо
 * действиям над указателями.
 *
 * Все функции библиотеки используют указатель на структуру uRpcHashTable.
 *
 * Создание хэш таблицы производится функцией #urpc_hash_table_create, удаление #urpc_hash_table_destroy.
 *
 * Основные операции с хэш таблицей:
 * - #urpc_hash_table_insert - добавление ключа/указателя в таблицу;
 * - #urpc_hash_table_find - поиск указателя по ключу;
 * - #urpc_hash_table_remove - удаление указателя по ключу.
 *
*/

#ifndef _urpc_hash_table_h
#define _urpc_hash_table_h

#include <urpc-exports.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct uRpcHashTable uRpcHashTable;


/*! Создание хэш таблицы.
 *
 * Функция создаёт пустую хэш таблицу.
 *
 * \return Указатель на хэш таблицу или NULL в случае ошибки.
 *
*/
URPC_EXPORT uRpcHashTable *urpc_hash_table_create( void );


/*! Удаление хэш таблицы.
 *
 * Функция удаляет все указатели из таблицы, а затем удаляет саму таблицу.
 *
 * \param uhash указатель на хэш таблицу.
 *
*/
URPC_EXPORT void urpc_hash_table_destroy( uRpcHashTable *uhash );


/*! Добавление ключа в хэш таблицу.
 *
 * Функция добавляет ключ и ассоциированный с этим ключом указатель в хэш таблицу.
 * Если указанный ключ уже существует, функция вернёт значение большее нуля. При этом
 * в таблице останется предыдущий указатель.
 *
 * \param uhash указатель на хэш таблицу;
 * \param key значение ключа;
 * \param value указатель на данные.
 *
 * \return 0 - если ключ добавлен, 1 - если ключ уже существует, -1 в случае ошибки.
 *
*/
URPC_EXPORT int urpc_hash_table_insert( uRpcHashTable *uhash, uint32_t key, void *value );


/*! Поиск указателя по ключу.
 *
 * Функция ищет в таблице ключ и возвращает указатель на данные.
 *
 * \param uhash указатель на хэш таблицу;
 * \param key значение ключа.
 *
 * \return Указатель на данные или NULL если такого ключа нет в таблице.
 *
 *
*/
URPC_EXPORT void *urpc_hash_table_find( uRpcHashTable *uhash, uint32_t key );


/*! Удаление ключа из таблицы.
 *
 * \param uhash указатель на хэш таблицу;
 * \param key значение ключа.
 *
 * \return 0 если ключ был удалён, 1 если такого ключа нет, -1 в случае ошибки.
 *
*/
URPC_EXPORT int urpc_hash_table_remove( uRpcHashTable *uhash, uint32_t key );


#ifdef __cplusplus
} // extern "C"
#endif

#endif // _urpc_hash_table_h
