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
 * \file urpc-mutex.h
 *
 * \brief Заголовочный файл библиотеки работы с мьютексами
 * \author Andrei Fadeev (andrei@webcontrol.ru)
 * \date 2015
 * \license GNU General Public License version 3 или более поздняя<br>
 * Коммерческая лицензия - свяжитесь с автором
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
 * Все функции библиотеки используют указатель на структуру uRpcMutex. Структура может быть
 * создана динамически или объявлена как статический объект. Перед использованием необходимо
 * инициализировать мьютекс функцией #urpc_mutex_init. Функция #urpc_mutex_clear освобождает
 * ресурсы выделенные под мьютекс при инициализации. Функцию urpc_mutex_clear можно использовать
 * только для разблокированного мьютекса.
 *
 * Доступно две функции блокировки мьютекса и одна разблокировки.
 *
 * #urpc_mutex_lock - функция безусловно пытается заблокировать мьютекс. Выход из функции возможен только
 * в случае успешной блокировки. #urpc_mutex_trylock - функция однократно пытается заблокировать мьютекс и
 * завершает свою работу независимо от результата. #urpc_mutex_unlock - функция разблокирует мьютекс.
 *
 */

#ifndef __URPC_MUTEX_H__
#define __URPC_MUTEX_H__

#include <urpc-exports.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32)
#include <windows.h>
typedef CRITICAL_SECTION uRpcMutex;
#endif

#if defined(__unix__)
#include <pthread.h>
typedef pthread_mutex_t uRpcMutex;
#endif

/**
 *
 * Функция инициализирует мьютекс.
 *
 * \param mutex указатель мьютекс.
 *
 * \return Нет.
 *
*/
URPC_EXPORT
void           urpc_mutex_init                 (uRpcMutex             *mutex);

/**
 *
 * Функция освобождает ресурсы выделенные для мьютекса.
 *
 * \param mutex указатель на мьютекс.
 *
 * \return Нет.
 *
*/
URPC_EXPORT
void           urpc_mutex_clear                (uRpcMutex             *mutex);

/**
 *
 * Функция безусловно пытается заблокировать мьютекс. Функция завершает свою работу
 * только в случае успешной блокировки.
 *
 * \param mutex казатель на мьютекс.
 *
 * \return Нет.
 *
*/
URPC_EXPORT
void           urpc_mutex_lock                 (uRpcMutex             *mutex);

/**
 *
 * Функция однократно пытается заблокировать мьютекс и завершает свою работу.
 *
 * \param mutex указатель на мьютекс.
 *
 * \return 0 - в случае успешной блокировки, иначе отрицательное число.
 *
*/
URPC_EXPORT
int            urpc_mutex_trylock              (uRpcMutex             *mutex);

/**
 *
 * Функция разблокирует мьютекс. Если мьютекс был заблокирован другим потоком
 * поведение функции не определено.
 *
 * \param mutex указатель на мьютекс.
 *
 * \return Нет.
 *
*/
URPC_EXPORT
void           urpc_mutex_unlock               (uRpcMutex             *mutex);

#ifdef __cplusplus
}
#endif

#endif /* __URPC_MUTEX_H__ */
