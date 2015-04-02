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
 * \file urpc-mutex.h
 *
 * \author Andrei Fadeev
 * \date 23.03.2015
 * \brief Заголовочный файл библиотеки работы с мьютексами.
 *
 * \defgroup uRpcMutex uRpcMutex - библиотека работы с мьютексами.
 *
 * Библиотека предназначена для кросплатформенной работы с мьютексами. В POSIX совместимых
 * системах используется pthread_mutex, в Windows системах используется CriticalSection.
 *
 * Работа с мьютексами возможна только в рамках одного процесса. Разблокировку мьютекса может
 * произвести только поток который заблокировал его. Если требуется возможность разблокировки
 * другими потоками следует использовать механизм семафоров - \link uRpcSemaphore \endlink.
 *
 * Все функции библиотеки используют указатель на структуру uRpcMutex.
 *
 * Создание мьютекса производится функцией #urpc_mutex_create, удаление #urpc_mutex_destroy.
 *
 * Доступно две функции блокировки мьютекса и одна разблокировки.
 *
 * #urpc_mutex_lock - функция безусловно пытается заблокировать мьютекс. Выход из функции возможен только
 * в случае успешной блокировки. #urpc_mutex_trylock - функция однократно пытается заблокировать мьютекс и
 * завершает свою работу независимо от результата. #urpc_mutex_unlock - функция разблокирует мьютекс.
 *
*/

#ifndef _urpc_mutex_h
#define _urpc_mutex_h

#include <urpc-exports.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct uRpcMutex uRpcMutex;


/*! Создание мьютекса.
 *
 * Функция создаёт мьютекс и возвращает указатель на него.
 *
 * \return Указатель на мьютекс или NULL в случае ошибки.
 *
*/
URPC_EXPORT uRpcMutex *urpc_mutex_create( void );


/*! Удаление мьютекса.
 *
 * Функция удаляет мьютекс и освобождает память занятую им.
 *
 * \param mutex указатель на мьютекс.
 *
 * \return Нет.
 *
*/
URPC_EXPORT void urpc_mutex_destroy( uRpcMutex *mutex );


/*! Блокировка мьютекса.
 *
 * Функция безусловно пытается заблокировать мьютекс. Функция завершает свою работу
 * только в случае успешной блокировки.
 *
 * \param mutex казатель на мьютекс.
 *
 * \return Нет.
 *
*/
URPC_EXPORT void urpc_mutex_lock( uRpcMutex *mutex );


/*! Блокировка мьютекса.
 *
 * Функция однократно пытается заблокировать мьютекс и завершает свою работу.
 *
 * \param mutex указатель на мьютекс.
 *
 * \return 0 - в случае успешной блокировки, иначе отрицательное число.
 *
*/
URPC_EXPORT int urpc_mutex_trylock( uRpcMutex *mutex );


/*! Разблокировка мьютекса.
 *
 * Функция разблокирует мьютекс. Если мьютекс был заблокирован другим потоком
 * поведение функции не определено.
 *
 * \param mutex указатель на мьютекс.
 *
 * \return Нет.
 *
*/
URPC_EXPORT void urpc_mutex_unlock( uRpcMutex *mutex );


#ifdef __cplusplus
} // extern "C"
#endif

#endif // _urpc_mutex_h
