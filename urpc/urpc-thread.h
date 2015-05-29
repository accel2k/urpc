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
 * \file urpc-thread.h
 *
 * \brief Заголовочный файл библиотеки работы с потоками
 * \author Andrei Fadeev (andrei@webcontrol.ru)
 * \date 2015
 * \license GNU General Public License version 3 или более поздняя<br>
 * Коммерческая лицензия - свяжитесь с автором
 *
 * \defgroup uRpcThread uRpcThread - библиотека работы с потоками.
 *
 * Библиотека предназначена для кросплатформенной работы с потоками. В POSIX совместимых
 * системах используется pthread_mutex, в Windows системах используется Win API Threads.
 *
 * Все функции библиотеки используют указатель на структуру uRpcThread.
 *
 * Выполнение потока осуществляется в рамках функции типа #urpc_thread_func. В функцию может быть
 * передан указатель на внешние данные. Поток завершается после выполнения в его функции оператора
 * return или вызова функции #urpc_thread_exit.
 *
 * Создание потока осуществляется функцией #urpc_thread_create. Родительский поток может узнать
 * о завершении дочернего потока с использованием функции #urpc_thread_join. Функция #urpc_thread_destroy
 * ожидает завершения потока и после этого освобождает память занятую управляющей структурой.
 *
*/

#ifndef _urpc_thread_h
#define _urpc_thread_h

#include <urpc-exports.h>

#ifdef __cplusplus
extern "C" {
#endif


#if defined( _WIN32 )
#include <windows.h>
typedef HANDLE uRpcThread;
#endif

#if defined( __unix__ )
#include <pthread.h>
typedef pthread_t uRpcThread;
#endif


/*! Тип функции запускаемой в качестве потока.
 *
 * \param data указатель на данные передаваемые в поток.
 *
 * \return Результат работы потока.
 *
*/
typedef void* (*urpc_thread_func)( void *data );


/*! Создание потока.
 *
 * Функция создаёт новый поток и запускает выполнение в нём пользовательской функции.
 *
 * \param func функция выполняемая в потоке;
 * \param data указатель на данные передаваемые в функцию потока.
 *
 * \return Указатель на поток или NULL.
 *
*/
URPC_EXPORT uRpcThread *urpc_thread_create( urpc_thread_func func, void *data );


/*! Удаление потока.
 *
 * функция ожидает завершения потока и освобождает память занятую управляющей структурой.
 *
 * \param thread указатель на структуру.
 *
 * \return Нет.
 *
*/
URPC_EXPORT void urpc_thread_destroy( uRpcThread *thread );


/*! Ожидание завершения потока.
 *
 * Функция ожидает завершения потока.
 *
 * \param thread указатель на структуру.
 *
 * \return Результат работы потока.
 *
*/
URPC_EXPORT void *urpc_thread_join( uRpcThread *thread );


/*! Завершение работы потока.
 *
 * Функция может быть вызвана только в дочернем потоке и преведет к
 * завершению его работы. Результат работы потока вернет функция #urpc_thread_join.
 *
 * \param retval результат работы потока.
 *
 * \return Нет.
 *
*/
URPC_EXPORT void urpc_thread_exit( void *retval );


#ifdef __cplusplus
} // extern "C"
#endif

#endif // _urpc_thread_h
