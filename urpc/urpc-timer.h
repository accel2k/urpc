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
 * \file urpc-timer.h
 *
 * \brief Заголовочный файл библиотеки работы с таймерами
 * \author Andrei Fadeev (andrei@webcontrol.ru)
 * \date 2015
 * \license GNU General Public License version 3 или более поздняя<br>
 * Коммерческая лицензия - свяжитесь с автором
 *
 * \defgroup uRpcTimer uRpcTimer - библиотека работы с таймерами.
 *
 * При помощи таймеров можно измерять промежутки времени. Создание таймера производится
 * функцией #urpc_timer_create. Во время создания запоминается начальный момент времени.
 * Функция #urpc_timer_elapsed возвращает значение интервала времени в секундах между
 * начальным и текущим моментами. Функция #urpc_timer_start запоминает новый начальный
 * момент времени.
 *
 * Удаление таймера производится функцией #urpc_timer_destroy.
 *
 * Функция urpc_timer_sleep может использоваться для остановки выполнения программы или
 * потока на заданную длительность времени.
 *
 */

#ifndef __URPC_TIMER_H__
#define __URPC_TIMER_H__

#include <urpc-exports.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _uRpcTimer uRpcTimer;

/**
 *
 * Функция создаёт таймер и запоминает начальное время.
 *
 * \return Указатель на таймер или NULL в случае ошибки.
 *
 */
URPC_EXPORT
uRpcTimer *urpc_timer_create   (void);

/**
 *
 * Функция удаляет таймер.
 *
 * \param timer указатель на таймер.
 *
 * \return Нет.
 *
 */
URPC_EXPORT
void urpc_timer_destroy        (uRpcTimer             *timer);

/**
 *
 * Функция запоминает начальный момент относительно которого функция
 * #urpc_timer_elapsed будет измерять интервал времени.
 *
 * \param timer указатель на таймер.
 *
 * \return Нет.
 *
 */
URPC_EXPORT
void urpc_timer_start          (uRpcTimer             *timer);

/**
 *
 * Функция возвращает значение интервала времени в секундах между начальним
 * и текущим моментами.
 *
 * \param timer указатель на таймер.
 *
 * \return Интервал времени в секундах или отрицательное число в случае ошибки.
 *
*/
URPC_EXPORT
double urpc_timer_elapsed      (uRpcTimer             *timer);

/**
 *
 * Функция останавливает выполнения программы на заданную длительность времени.
 *
 * \param time длительность остановки в секундах.
 *
 * \return Нет.
 *
*/
URPC_EXPORT
void urpc_timer_sleep          (double                 time);

#ifdef __cplusplus
}
#endif

#endif /* __URPC_TIMER_H__ */
