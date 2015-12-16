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
 * \file urpc-shm.h
 *
 * \brief Заголовочный файл библиотеки работы с разделяемой памятью
 * \author Andrei Fadeev (andrei@webcontrol.ru)
 * \date 2015
 * \license GNU General Public License version 3 или более поздняя<br>
 * Коммерческая лицензия - свяжитесь с автором
 *
 * \defgroup uRpcShm uRpcShm - библиотека работы с разделяемой памятью.
 *
 * Библиотека предназначена для кросплатформенной работы с разделяемой памятью. В POSIX совместимых
 * системах используется shm_open/mmap, в Windows системах используется CreateFileMapping/MapViewOfFile.
 *
 * Работа с разделяемой памятью возможна из нескольких процессов. Для обеспечения целостности данных
 * при совместном доступе следует использовать семафоры - \link uRpcSemaphore \endlink.
 *
 * Все функции библиотеки используют указатель на структуру uRpcShm.
 *
 * Создание сегмента с разделяемой памятью осуществляется функцией #urpc_shm_create, открытие уже существующего
 * сегмента осуществляется функцией #urpc_shm_open или #urpc_shm_open_ro, удаление сегмента осуществляется функцией #urpc_shm_destroy.
 *
 * В Unix системах созданный сегмент остаётся в качестве объекта и после закрытия всех указателей на него.
 * Для его удаления используется функция #urpc_shm_remove. В Windows эта функция не выполняет никаких действий.
 * Функция #urpc_shm_destroy самостоятельно удалит объект разделяемой памяти если он был создан функцией #urpc_shm_create.
 *
 * После создания сегмента разделяемой памяти необходимо выполнить отображение его в адресное пространство процесса.
 * Это действие выполняется функцией #urpc_shm_map.
 *
 */

#ifndef __URPC_SHM_H__
#define __URPC_SHM_H__

#include <urpc-exports.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _uRpcShm uRpcShm;

/**
 *
 * Функция создаёт сегмент разделяемой памяти и возвращает указатель на него.
 *
 * \param name название сегмента разделяемой памяти;
 * \param size размер сегмента в байтах.
 *
 * \return Указатель на сегмент разделяемой памяти или NULL в случае ошибки.
 *
 */
URPC_EXPORT
uRpcShm *urpc_shm_create       (const char            *name,
                                unsigned long          size);

/**
 *
 * Функция открывает сегмент разделяемой памяти в режиме чтения/записи и возвращает указатель на него.
 *
 * \param name название сегмента разделяемой памяти;
 * \param size размер сегмента в байтах.
 *
 * \return Указатель на сегмент разделяемой памяти или NULL в случае ошибки.
 *
 */
URPC_EXPORT
uRpcShm *urpc_shm_open         (const char            *name,
                                unsigned long          size);

/**
 *
 * Функция открывает сегмент разделяемой памяти в режиме только чтения и возвращает указатель на него.
 *
 * \param name название сегмента разделяемой памяти;
 * \param size размер сегмента в байтах.
 *
 * \return Указатель на сегмент разделяемой памяти или NULL в случае ошибки.
 *
*/
URPC_EXPORT
uRpcShm *urpc_shm_open_ro      (const char            *name,
                                unsigned long          size);

/**
 *
 * Функция удаляет сегмент разделяемой памяти и освобождает память занятую им.
 *
 * \param shm указатель на сегмент разделяемой памяти.
 *
 * \return Нет.
 *
*/
URPC_EXPORT
void urpc_shm_destroy          (uRpcShm               *shm);

/**
 *
 * Функция удаляет объект операционной системы связанный с сегментом разделяемой памяти.
 * В случае удаления сегмента разделяемой памяти функцией #urpc_shm_destroy объект удаляется
 * автоматически.  Данная функция необходима для удаления объекта оставшегося после аварийного
 * завершения программы.
 *
 * \param name название сегмента разделяемой памяти.
 *
 * \return Нет.
 *
 */
URPC_EXPORT
void urpc_shm_remove           (const char            *name);

/**
 *
 * Функция отображает сегмент раделяемой памяти в адресное пространство процесса.
 *
 * \param shm указатель на сегмент разделяемой памяти.
 *
 * \return адрес в пространстве процесса связанный с разделяемой памятью или NULL в случае ошибки.
 *
 */
URPC_EXPORT
void *urpc_shm_map             (uRpcShm               *shm);

#ifdef __cplusplus
}
#endif

#endif /* __URPC_SHM_H__ */
