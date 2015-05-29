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

/*!
 * \file urpc-hash-table.h
 *
 * \brief Заголовочный файл библиотеки работы с хэш таблицей
 * \author Andrei Fadeev (andrei@webcontrol.ru)
 * \date 2015
 * \license GNU General Public License version 3 или более поздняя<br>
 * Коммерческая лицензия - свяжитесь с автором
 *
 * \defgroup uRpcHashTable uRpcHashTable - библиотека работы с хэш таблицей.
 *
 * Хэш таблица предстваляет собой упрощённый вариант и предназначена для хранения указателей на
 * объекты общим числом примерно до 100000 элементов. Большее число элементов может размещаться в
 * таблице, но при этом возрастают накладные расходы на добавление и поиск.
 *
 * Хэш таблица сохраняет указатели на данные ассоциированные с ключами и по запросу возвращает эти
 * указатели. При удалении ключа или всей таблицы целиком может быть вызвана функция #urpc_hash_table_destroy_callback
 * для выполнения действий связанных с освобождением памяти выделенной для значения. Вызываемая функция
 * указывается при создании таблицы.
 *
 * Все функции библиотеки используют указатель на структуру uRpcHashTable.
 *
 * Создание хэш таблицы производится функцией #urpc_hash_table_create, удаление #urpc_hash_table_destroy.
 *
 * Основные операции с хэш таблицей:
 * - #urpc_hash_table_insert - добавление ключа/указателя в таблицу;
 * - #urpc_hash_table_insert_uint32 - добавление ключа/значения в таблицу;
 * - #urpc_hash_table_find - поиск указателя по ключу;
 * - #urpc_hash_table_find_uint32 - поиск значения по ключу;
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


/*! Тип функции вызываемой при удалении ключа из массива.
 *
 * Функция используется для освобождения памяти выделенной для значения ключа.
 *
 * \param data указатель на значение удаляемого ключа.
 *
 * \return Нет.
 *
*/
typedef void (*urpc_hash_table_destroy_callback)( void *data );


/*! Тип функции вызываемой при обходе всех ключей массива.
 *
 * Функция используется для выполнения определённых действий над всеми ключами массива.
 * В этой функции нельзя удалять элементы массива отличные от текущего обрабатываемого.
 * Если пользователь добавил новые элементы массива, нет гарантии, что они будут обработаны
 * в текущем цикле функции #urpc_hash_table_foreach.
 *
 * \param key значение ключа;
 * \param value указатель на данные;
 * \param user_data указатель на пользовательские данные.
 *
 * \return Нет.
 *
*/
typedef void (*urpc_hash_table_foreach_callback)( uint32_t key, void *value, void *user_data );


/*! Создание хэш таблицы.
 *
 * Функция создаёт пустую хэш таблицу.
 *
 * \param value_destroy_func функция особождения памяти значения или NULL.
 *
 * \return Указатель на хэш таблицу или NULL в случае ошибки.
 *
*/
URPC_EXPORT uRpcHashTable *urpc_hash_table_create( urpc_hash_table_destroy_callback value_destroy_func );


/*! Удаление хэш таблицы.
 *
 * Функция удаляет все указатели из таблицы, а затем удаляет саму таблицу.
 *
 * \param hash_table указатель на хэш таблицу.
 *
 * \return Нет.
 *
*/
URPC_EXPORT void urpc_hash_table_destroy( uRpcHashTable *hash_table );


/*! Добавление ключа в хэш таблицу.
 *
 * Функция добавляет ключ и ассоциированный с этим ключом указатель в хэш таблицу.
 * Если указанный ключ уже существует, функция вернёт значение большее нуля. При этом
 * в таблице останется предыдущий указатель.
 *
 * \param hash_table указатель на хэш таблицу;
 * \param key значение ключа;
 * \param value указатель на данные.
 *
 * \return 0 - если ключ добавлен, 1 - если ключ уже существует, -1 в случае ошибки.
 *
*/
URPC_EXPORT int urpc_hash_table_insert( uRpcHashTable *hash_table, uint32_t key, void *value );


/*! Добавление ключа в хэш таблицу.
 *
 * Функция добавляет ключ и ассоциированное с этим ключом значение типа uint32 в хэш таблицу.
 * Если указанный ключ уже существует, функция вернёт значение большее нуля. При этом
 * в таблице останется предыдущее значение.
 *
 * \param hash_table указатель на хэш таблицу;
 * \param key значение ключа;
 * \param value указатель на данные.
 *
 * \return 0 - если ключ добавлен, 1 - если ключ уже существует, -1 в случае ошибки.
 *
*/
URPC_EXPORT int urpc_hash_table_insert_uint32( uRpcHashTable *hash_table, uint32_t key, uint32_t value );


/*! Поиск указателя по ключу.
 *
 * Функция ищет в таблице ключ и возвращает указатель на данные.
 *
 * \param hash_table указатель на хэш таблицу;
 * \param key значение ключа.
 *
 * \return Указатель на данные или NULL если такого ключа нет в таблице.
 *
 *
*/
URPC_EXPORT void *urpc_hash_table_find( uRpcHashTable *hash_table, uint32_t key );


/*! Поиск значения по ключу.
 *
 * Функция ищет в таблице ключ и возвращает значение типа uint32. По результату
 * возвращаемому этой функцией нельзя дать однозначный ответ о присутствии ключа
 * в таблице. Для этого рекомендуется воспользоваться функцией #urpc_hash_table_find.
 *
 * \param hash_table указатель на хэш таблицу;
 * \param key значение ключа.
 *
 * \return значение ассоциированное с ключом или 0 если такого ключа нет в таблице.
 *
 *
*/
URPC_EXPORT uint32_t urpc_hash_table_find_uint32( uRpcHashTable *hash_table, uint32_t key );


/*! Обработка всех элементов массива.
 *
 * Функция вызывает для каждого ключа в массиве пользовательскую функцию #urpc_hash_table_foreach_callback.
 *
 * \param hash_table указатель на хэш таблицу;
 * \param callback пользовательская функция;
 * \param user_data пользовательские данные.
 *
 * \return Нет.
 *
*/
URPC_EXPORT void urpc_hash_table_foreach( uRpcHashTable *hash_table, urpc_hash_table_foreach_callback callback, void *user_data );


/*! Размер таблицы.
 *
 * Функция возвращает число элементов размещённых в таблице.
 *
 * \param hash_table указатель на хэш таблицу.
 *
 * \return Число элементов в таблице.
 *
*/
URPC_EXPORT uint32_t urpc_hash_table_size( uRpcHashTable *hash_table );


/*! Удаление ключа из таблицы.
 *
 * \param hash_table указатель на хэш таблицу;
 * \param key значение ключа.
 *
 * \return 0 если ключ был удалён, 1 если такого ключа нет, -1 в случае ошибки.
 *
*/
URPC_EXPORT int urpc_hash_table_remove( uRpcHashTable *hash_table, uint32_t key );


#ifdef __cplusplus
} // extern "C"
#endif

#endif // _urpc_hash_table_h
