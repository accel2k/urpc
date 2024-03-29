/*
 * uRPC - rpc (remote procedure call) library.
 *
 * Copyright 2009-2015 Andrei Fadeev (andrei@webcontrol.ru)
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
 * \file urpc-server.h
 *
 * \brief Заголовочный файл сервера удалённых вызовов процедур
 * \author Andrei Fadeev (andrei@webcontrol.ru)
 * \date 2009, 2010, 2014, 2015
 * \license GNU General Public License version 3 или более поздняя<br>
 * Коммерческая лицензия - свяжитесь с автором
 *
 * \defgroup uRpcServer uRpcServer - Сервер библиотеки удалённых вызовов процедур.
 *
 * Библиотека реализует механизм передачи rpc запросов от клиента к серверу через
 * определённый канал связи, такой как TCP, UDP или разделяемую область памяти.
 *
 * Сервер uRpc создается функцией #urpc_server_create, которая в случае успеха возвращает
 * указатель на новый объект. Этот объект может использоваться для настройки параметров RPC
 * и управления работой сервера.
 *
 * Перед началом работы необходимо произвести настройку параметров RPC и произвести запуск
 * сервера. Настройка параметров RPC включает в себя определение механизма безопасности соединения
 * с сервером и определения исполняемых процедур. Для этих целей используются следующие функции:
 *
 * - #urpc_server_set_security - выбор механизма аутентификации (и шифрования) данных;
 * - #urpc_server_set_server_key - задание секретного ключа цифровой подписи (шифрования) сервера;
 * - #urpc_server_add_client_key - добавление публичного ключа цифровой подписи (шифрования) клиента;
 * - #urpc_server_add_thread_start_callback - добавление callback функции вызываемой при запуске потока обработки;
 * - #urpc_server_add_thread_stop_callback - добавление callback функции вызываемой при остановке потока обработки;
 * - #urpc_server_add_connect_callback - добавление callback функции вызываемой при подключении клиента;
 * - #urpc_server_add_disconnect_callback - добавление callback функции вызываемой при отключении клиента;
 * - #urpc_server_add_callback - добаление callback функции исполняемой процедуры.
 *
 * Подробнее механизмы безопасности описаны в разделе \link uRpcSecurity \endlink.
 *
 * После настройки параметров RPC необходимо произвести запуск сервера с использованием
 * функции #urpc_server_bind. Для сервера создается отдельный поток (или потоки) в которых
 * будут выполняться все запросы от клиентов.
 *
 * Процедуры исполняемые сервером определяются при помощи числовых идентификаторов. Для пользователя
 * доступны идентификаторы с номерами больше чем \link URPC_PROC_USER \endlink. Например:
 *
 * - \#define USER_SIN_PROC URPC_PROC_USER + 1
 * - \#define USER_COS_PROC URPC_PROC_USER + 2
 *
 * Регистрация процедур возможна только до запуска uRCP сервера Для работы необходимо зарегистрировать
 * callback функции, которые будут вызываться при поступлении запроса с совпадающим идентификатором proc_id.
 *
 * Таким образом если зарегистрировать процедуры proc1 с идентификатором PROC_ID1 и proc2 с идентификатором
 * PROC_ID2, то при RPC запросе #urpc_client_exec ( rpc, PROC_ID1 ) на сервере выполнится процедура proc1.
 * А при RPC запросе #urpc_client_exec ( rpc, PROC_ID2 ) на сервере выполнится функция proc2.
 *
 * Удаление сервера производится функцией #urpc_server_destroy.
 *
 */

#ifndef __URPC_SERVER_H__
#define __URPC_SERVER_H__

#include <urpc-exports.h>
#include <urpc-types.h>
#include <urpc-data.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _uRpcServer uRpcServer;

/**
 *
 * Тип функции вызываемой при отключении клиента или остановки потока обработки.
 * В неё передаётся указатель на область памяти выделенный при подключении
 * клиента или запуска потока обработки. Функция должна освободить эту
 * память.
 *
 * \param urpc_data данные (#urpc_server_add_thread_start_callback, urpc_server_add_connect_callback);
 * \param user_data пользовательские данные (#urpc_server_add_thread_stop_callback, #urpc_server_add_disconnect_callback).
 *
 * \return Нет.
 *
 */
typedef void (*urpc_free_proc)                 (void                  *urpc_data,
                                                void                  *user_data);

/**
 *
 * Тип функции вызываемой при создании потока обработки запросов клиентов.
 * В этой функции можно выделить область памяти и сохранить в ней данные
 * необходимые для потоков обработки запросов.
 *
 * При заврешении потока вызывается функция #urpc_free_proc, в которую
 * передаётся указатель на данные из этой функции.
 *
 * \param user_data пользовательские данные (#urpc_server_add_thread_start_callback).
 *
 * \return Данные потока обработки запросов.
 *
 */
typedef void * (*urpc_thread_proc)             (void                  *user_data);

/**
 *
 * Тип функции вызываемой при подключении клиента. В этой функции можно
 * выделить область памяти и сохранить в ней параметры подключения, необходимые
 * пользователю для работы.
 *
 * При отключении пользователя вызывается функция #urpc_free_proc, в которую
 * передаётся указатель на данные из этой функции.
 *
 * \param session идентификатор сессии клиента;
 * \param key_data указатель на данные связанные с публичным ключом клиента;
 * \param user_data пользовательские данные (#urpc_server_add_connect_callback).
 *
 * \return Данные сессии клиента.
 *
 */
typedef void * (*urpc_connect_proc)            (uint32_t               session,
                                                void                  *key_data,
                                                void                  *user_data);

/**
 *
 * Тип функции вызываемой при получении запроса на выполнение. Данные запроса
 * клиента доступны для чтения через объект \link uRpcData \endlink. Данные
 * ответа должны быть записаны через этот же объект. Если серверная процедура
 * успешно выполнена функция должна вернуть значение ноль. Отрицательное
 * значение возвращается в случае ошибок при вызове процедуры, например если
 * переданы не все параметры.
 *
 * \param urpc_data указатель на объект \link uRpcData \endlink;
 * \param thread_data указатель на данные связанные с потоком испольнения;
 * \param session_data указатель на данные связанные с сессией клиента;
 * \param user_data пользовательские данные (#urpc_server_add_callback).
 *
 * \return 0 если RPC запрос был выполнен, иначе отрицательное число.
 *
 */
typedef int (*urpc_proc)                       (uRpcData              *urpc_data,
                                                void                  *thread_data,
                                                void                  *session_data,
                                                void                  *user_data);

/**
 *
 * Функция создает RPC сервер по адресу uri. Адрес задается в виде строки:
 * "<type>://name:port", где:
 * - &lt;type&gt; - тип RPC (udp, tcp, shm);
 * - name - имя или ip адрес системы;
 * - port - номер udp или tcp порта.
 *
 * Для IP версии 6 ip адрес должен быть задан в прямых скобках [], например [::1/128].
 * Для shm номер порта может быть любым или отсутствовать.
 *
 * \param uri адрес сервера;
 * \param threads_num число потоков исполнения на сервере;
 * \param max_clients максимальное число клиентов подключенных к серверу;
 * \param session_timeout интервал времени отключения неактивных клиентов;
 * \param max_data_size размер буфера приема-передачи в байтах;
 * \param data_timeout максимальное время задержки при передаче данных.
 *
 * \return Указатель на uRpcServer объект в случае успеха, иначе NULL.
 *
 */
URPC_EXPORT
uRpcServer *urpc_server_create                 (const char            *uri,
                                                uint32_t               threads_num,
                                                uint32_t               max_clients,
                                                double                 session_timeout,
                                                uint32_t               max_data_size,
                                                double                 data_timeout);

/**
 *
 * Функция завершает потоки исполнения и удаляет RPC сервер.
 *
 * \param urpc_server указатель на uRpcServer объект.
 *
 * \return Нет.
 *
 */
URPC_EXPORT
void urpc_server_destroy                       (uRpcServer            *urpc_server);

/**
 *
 * Функция определяет механизм безопасности используемый для взаимодействия с
 * сервером. По умолчанию для взаимодействия с сервером не используется никаких
 * механизмов аутентификации и шифрования.
 *
 * \param urpc_server указатель на uRpcServer объект;
 * \param mode режим безопасности.
 *
 * \return 0 если режим безопасности успешно установлен, отрицательное число в случае ошибки.
 *
 */
URPC_EXPORT
int urpc_server_set_security                   (uRpcServer            *urpc_server,
                                                uRpcSecurity           mode);

/**
 *
 * Функция задаёт ключ используемый сервером для аутентификации ответов клиенту.
 *
 * \param urpc_server указатель на uRpcServer объект;
 * \param priv_key указатель на секретный серверный ключ.
 *
 * \return 0 если ключ успешно задан, отрицательное число в случае ошибки.
 *
 */
URPC_EXPORT
int urpc_server_set_server_key                 (uRpcServer            *urpc_server,
                                                const unsigned char   *priv_key);

/**
 *
 * Функция добавляет ключ используемый для аутентификации клиента сервером. Сервер
 * может хранить несколько ключей аутентификации. При вызове процедуры клиентом
 * аутентифицированном этим ключом в callback функцию будет передан указатель на
 * данные key_data.
 *
 * \param urpc_server указатель на uRpcServer объект;
 * \param pub_key указатель на публичный клиентский ключ;
 * \param key_data дополнительные данные связанные с этим ключом.
 *
 * \return 0 если ключ успешно задан, отрицательное число в случае ошибки.
 *
 */
URPC_EXPORT
int urpc_server_add_client_key                 (uRpcServer            *urpc_server,
                                                const unsigned char   *pub_key,
                                                void                  *key_data);

/**
 *
 * Функция регистрирует callback функцию вызываемую при запуске потока обработки
 * запросов клиентов.
 *
 * \param urpc_server указатель на uRpcServer объект;
 * \param proc callback функция;
 * \param data пользовательские данные.
 *
 * \return 0 в случае успешного завершения, иначе отрицательное значение.
 *
 */
URPC_EXPORT
int urpc_server_add_thread_start_callback      (uRpcServer            *urpc_server,
                                                urpc_thread_proc       proc,
                                                void                  *data);

/**
 *
 * Функция регистрирует callback функцию вызываемую при остановке потока обработки
 * запросов клиентов.
 *
 * \param urpc_server указатель на uRpcServer объект;
 * \param proc callback функция;
 * \param data пользовательские данные.
 *
 * \return 0 в случае успешного завершения, иначе отрицательное значение.
 *
 */
URPC_EXPORT
int urpc_server_add_thread_stop_callback       (uRpcServer            *urpc_server,
                                                urpc_free_proc         proc,
                                                void                  *data);

/**
 *
 * Функция регистрирует callback функцию вызываемую при подключении клиента.
 *
 * \param urpc_server указатель на uRpcServer объект;
 * \param proc callback функция;
 * \param data пользовательские данные.
 *
 * \return 0 в случае успешного завершения, иначе отрицательное значение.
 *
 */
URPC_EXPORT
int urpc_server_add_connect_callback           (uRpcServer            *urpc_server,
                                                urpc_connect_proc      proc,
                                                void                  *data);

/**
 *
 * Функция регистрирует callback функцию вызываемую при отключении клиента.
 *
 * \param urpc_server указатель на uRpcServer объект;
 * \param proc callback функция;
 * \param data пользовательские данные.
 *
 * \return 0 в случае успешного завершения, иначе отрицательное значение.
 *
 */
URPC_EXPORT
int urpc_server_add_disconnect_callback        (uRpcServer            *urpc_server,
                                                urpc_free_proc         proc,
                                                void                  *data);

/**
 *
 * Функция регистрирует callback функцию с идентификатором id. Эта функция будет
 * вызвана когда клиент вызовет #urpc_client_exec с соответствующим id. Все
 * переданные параметры доступны в callback функции через объект
 * \link uRpcData \endlink. Результат работы должен быть передан обратно
 * регистрацией переменных через объект \link uRpcData \endlink.
 *
 * \param urpc_server указатель на uRpcServer объект;
 * \param proc_id идентификатор callback функции;
 * \param proc callback функция;
 * \param data пользовательские данные.
 *
 * \return 0 в случае успешного завершения, иначе отрицательное значение.
 *
 */
URPC_EXPORT
int urpc_server_add_callback                   (uRpcServer            *urpc_server,
                                                uint32_t               proc_id,
                                                urpc_proc              proc,
                                                void                  *data);

/**
 *
 * Функция производит запуск сервера с использованием выбранного механизма
 * безопасности и зарегистрированными процедурами.
 *
 * \param urpc_server указатель на uRpcServer объект;
 *
 * \return 0 в случае успешного запуска, иначе отрицательное значение.
 *
 */
URPC_EXPORT
int urpc_server_bind                           (uRpcServer            *urpc_server);

#ifdef __cplusplus
}
#endif

#endif /* __URPC_SERVER_H__ */
