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
 * \file urpc-semaphore.h
 *
 * \brief Заголовочный файл библиотеки работы с семафорами
 * \author Andrei Fadeev (andrei@webcontrol.ru)
 * \date 2015
 * \license GNU General Public License version 3 или более поздняя<br>
 * Коммерческая лицензия - свяжитесь с автором
 *
 * \defgroup uRpcSemaphore uRpcSemaphore - библиотека работы с семафорами.
 *
 * Библиотека предназначена для кросплатформенной работы с семафорами. В POSIX совместимых системах
 * используется sem_open/sem_wait, в Windows системах используется CreateSemaphore/WaitForSingleObject.
 *
 * Работа с семафорами возможна из нескольких процессов. Разблокировку семафора может произвести процесс
 * отличный от установившего блокировку. Семафоры могут содержать очередь использующих процессов,
 * таким образом становится возможным блокировка одного семафора несколькими процессами. Размер очереди
 * указывается во время создания семафора.
 *
 * Все функции библиотеки используют указатель на структуру uRpcSem.
 *
 * Создание семафора производится функцией #urpc_sem_create, открытие существующего функцией #urpc_sem_open,
 * удаление семафора производится функцией #urpc_sem_destroy.
 *
 * В Unix системах созданный семафор остаётся в качестве объекта и после закрытия всех указателей на него.
 * Для его удаления используется функция #urpc_sem_remove. В Windows эта функция не выполняет никаких действий.
 * Функция #urpc_sem_destroy самостоятельно удалит объект семафора если он был создан функцией #urpc_sem_create.
 *
 * Доступно три функции блокировки семафора и одна разблокировки.
 *
 * #urpc_sem_lock - функция безусловно пытается заблокировать семафор. Выход из функции возможен только
 * в случае успешной блокировки. #urpc_sem_trylock - функция однократно пытается заблокировать семафор и
 * завершает свою работу независимо от результата. #urpc_sem_timedlock - функция пытается заблокировать
 * семафор в течение указанного времени. #urpc_sem_unlock - функция разблокирует семафор.
 *
*/

#ifndef _urpc_semaphore_h
#define _urpc_semaphore_h

#include <urpc-exports.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef enum { URPC_SEM_LOCKED = 0, URPC_SEM_UNLOCKED = 1 } uRpcSemStat;

typedef struct uRpcSem uRpcSem;


/*! Создание семафора.
 *
 * Функция создаёт семафор и возвращает указатель на него. При создании семафора
 * можно указать его начальное состояние URPC_SEM_LOCKED - семафор заблокирован или
 * URPC_SEM_UNLOCKED - семафор разблокирован и размер очереди блокировок.
 *
 * \param name название семафора;
 * \param stat состояние семафора после создания;
 * \param queue размер очереди блокировок.
 *
 * \return Указатель на семафор или NULL в случае ошибки.
 *
*/
URPC_EXPORT uRpcSem *urpc_sem_create( const char *name, uRpcSemStat stat, int queue );


/*! Открытие существующего семафора.
 *
 * Функция открывает уже созданный семафор и возвращает указатель на него. Функция не
 * изменяет текущего состояния семафора.
 *
 * \param name название семафора.
 *
 * \return Указатель на семафор или NULL в случае ошибки.
 *
*/
URPC_EXPORT uRpcSem *urpc_sem_open( const char *name );


/*! Удаление семафора.
 *
 * Функция удаляет семафор и освобождает память занятую им.
 *
 * \param sem указатель на семафор.
 *
 * \return Нет.
 *
*/
URPC_EXPORT void urpc_sem_destroy( uRpcSem *sem );


/*! Удаление объекта семафора.
 *
 * Функция удаляет объект операционной системы связанный с семафором.
 * В случае удаления семафора функцией #urpc_sem_destroy объект удаляется автоматически.
 * Данная функция необходима для удаления объекта оставшегося после аварийного завершения
 * программы.
 *
 * \param name название семафора.
 *
 * \return Нет.
 *
*/
URPC_EXPORT void urpc_sem_remove( const char *name );


/*! Блокировка семафора.
 *
 * Функция безусловно пытается заблокировать семафор. Функция завершает свою работу
 * только в случае успешной блокировки.
 *
 * \param sem казатель на семафор.
 *
 * \return Нет.
 *
*/
URPC_EXPORT void urpc_sem_lock( uRpcSem *sem );


/*! Блокировка семафора.
 *
 * Функция однократно пытается заблокировать семафор и завершает свою работу.
 *
 * \param sem казатель на семафор.
 *
 * \return 0 - в случае успешной блокировки, иначе отрицательное число.
 *
*/
URPC_EXPORT int urpc_sem_trylock( uRpcSem *sem );


/*! Блокировка семафора.
 *
 * Функция пытается заблокировать семафор в течение времени. Если в течение указанного времени
 * блокировка была получена функция завершает свою работу немедленно.
 *
 * \param sem казатель на семафор;
 * \param time время в течение которого необходимо получить блокировку.
 *
 * \return 0 - в случае успешной блокировки, положительное число в случае таймаута, иначе отрицательное число.
 *
*/
URPC_EXPORT int urpc_sem_timedlock( uRpcSem *sem, double time );


/*! Разблокировка семафора.
 *
 * Функция разблокирует семафор.
 *
 * \param sem указатель на семафор.
 *
 * \return Нет.
 *
*/
URPC_EXPORT void urpc_sem_unlock( uRpcSem *sem );


#ifdef __cplusplus
} // extern "C"
#endif

#endif // _urpc_semaphore_h
