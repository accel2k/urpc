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
 * \file urpc-rwmutex.h
 *
 * \brief Заголовочный файл библиотеки работы с разделяемыми мьютексами
 * \author Andrei Fadeev (andrei@webcontrol.ru)
 * \date 2015
 * \copyright GNU General Public License version 3 or later
 *
 * \defgroup uRpcRWMutex uRpcRWMutex - библиотека работы с разделяемыми мьютексами.
 *
 * Библиотека предназначена для кросплатформенной работы с разделяемыми мьютексами позволяющими
 * выполнять параллельное чтение данных нескольким потокам и однопоточную запись. В POSIX совместимых
 * системах используется pthread_rwlock, в Windows системах используется SRWLOCK.
 *
 * Все функции библиотеки используют указатель на структуру uRpcRWMutex. Структура может быть
 * создана динамически или объявлена как статический объект. Перед использованием необходимо
 * инициализировать мьютекс функцией #urpc_rwmutex_init. Функция #urpc_rwmutex_clear освобождает
 * ресурсы выделенные под мьютекс при инициализации. Функцию urpc_rwmutex_clear можно использовать
 * только для разблокированного мьютекса.
 *
 * Доступно две функции блокировки мьютекса и одна разблокировки.
 *
 * #urpc_mutex_lock - функция безусловно пытается заблокировать мьютекс. Выход из функции возможен только
 * в случае успешной блокировки. #urpc_mutex_trylock - функция однократно пытается заблокировать мьютекс и
 * завершает свою работу независимо от результата. #urpc_mutex_unlock - функция разблокирует мьютекс.
 *
*/

#ifndef _urpc_rwmutex_h
#define _urpc_rwmutex_h

#include <urpc-exports.h>

#ifdef __cplusplus
extern "C" {
#endif


#if defined( _WIN32 )
#include <windows.h>
typedef SRWLOCK uRpcRWMutex;
#endif

#if defined( __unix__ )
#include <pthread.h>
typedef pthread_rwlock_t uRpcRWMutex;
#endif


/*! Инициализация мьютекса.
 *
 * Функция инициализирует мьютекс.
 *
 * \param rwmutex указатель мьютекс.
 *
 * \return Нет.
 *
*/
URPC_EXPORT void urpc_rwmutex_init( uRpcRWMutex *rwmutex );


/*! Удаление мьютекса.
 *
 * Функция освобождает ресурсы выделенные для мьютекса.
 *
 * \param rwmutex указатель на мьютекс.
 *
 * \return Нет.
 *
*/
URPC_EXPORT void urpc_rwmutex_clear( uRpcRWMutex *rwmutex );


/*! Блокировка мьютекса для чтения.
 *
 * Функция безусловно пытается заблокировать мьютекс для чтения. Функция завершает свою работу
 * только в случае успешной блокировки.
 *
 * \param rwmutex казатель на мьютекс.
 *
 * \return Нет.
 *
*/
URPC_EXPORT void urpc_rwmutex_reader_lock( uRpcRWMutex *rwmutex );


/*! Блокировка мьютекса для чтения.
 *
 * Функция однократно пытается заблокировать мьютекс для чтения и завершает свою работу.
 *
 * \param rwmutex указатель на мьютекс.
 *
 * \return 0 - в случае успешной блокировки, иначе отрицательное число.
 *
*/
URPC_EXPORT int urpc_rwmutex_reader_trylock( uRpcRWMutex *rwmutex );


/*! Разблокировка мьютекса.
 *
 * Функция разблокирует мьютекс заблокированный для чтения. Если мьютекс был заблокирован
 * другим потоком поведение функции не определено.
 *
 * \param rwmutex указатель на мьютекс.
 *
 * \return Нет.
 *
*/
URPC_EXPORT void urpc_rwmutex_reader_unlock( uRpcRWMutex *rwmutex );


/*! Блокировка мьютекса для записи.
 *
 * Функция безусловно пытается заблокировать мьютекс для записи. Функция завершает свою работу
 * только в случае успешной блокировки.
 *
 * \param rwmutex казатель на мьютекс.
 *
 * \return Нет.
 *
*/
URPC_EXPORT void urpc_rwmutex_writer_lock( uRpcRWMutex *rwmutex );


/*! Блокировка мьютекса для записи.
 *
 * Функция однократно пытается заблокировать мьютекс для записи и завершает свою работу.
 *
 * \param rwmutex указатель на мьютекс.
 *
 * \return 0 - в случае успешной блокировки, иначе отрицательное число.
 *
*/
URPC_EXPORT int urpc_rwmutex_writer_trylock( uRpcRWMutex *rwmutex );


/*! Разблокировка мьютекса.
 *
 * Функция разблокирует мьютекс заблокируемый для записи. Если мьютекс был заблокирован
 * другим потоком поведение функции не определено.
 *
 * \param rwmutex указатель на мьютекс.
 *
 * \return Нет.
 *
*/
URPC_EXPORT void urpc_rwmutex_writer_unlock( uRpcRWMutex *rwmutex );


#ifdef __cplusplus
} // extern "C"
#endif

#endif // _urpc_rwmutex_h
