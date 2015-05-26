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

/*!
 * \file urpc-client.h
 *
 * \brief Заголовочный файл клиента удалённых вызовов процедур
 * \author Andrei Fadeev (andrei@webcontrol.ru)
 * \date 2009-2015
 * \copyright GNU General Public License version 3 or later
 *
 * \defgroup uRpcClient uRpcClient - Клиент библиотеки удалённых вызовов процедур.
 *
 * Библиотека реализует механизм передачи rpc запросов от клиента к серверу через
 * определённый канал связи, такой как TCP, UDP или разделяемую область памяти.
 *
 * Клиент uRpc создается функцией #urpc_client_create, которая возвращает указатель
 * на новый объект. Этот объект может использоваться для настройки параметров RPC,
 * передачи запросов и получения результатов их исполнения сервером.
 *
 * Перед началом взаимодействия с сервером необходимо произвести настройку параметров RPC
 * и произвести подключение к серверу. Настройка параметров RPC включает в себя
 * определение механизма безопасности соединения с сервером. Для этих целей используются
 * следующие функции:
 *
 * - #urpc_client_set_security - выбор механизма аутентификации (и шифрования) данных;
 * - #urpc_client_set_client_key - задание секретного ключа цифровой подписи (шифрования) клиента;
 * - #urpc_client_set_server_key - задание публичного ключа цифровой подписи (шифрования) сервера.
 *
 * Подробнее механизмы безопасности описаны в разделе \link uRpcSecurity \endlink.
 *
 * После определения параметров безопасности необходимо произвести подключение к серверу
 * с использованием функции #urpc_client_connect.
 *
 * Взаимодействие клиента сервером происходит следующим образом:
 *
 * - клиент блокирует канал передачи данных функцией #urpc_client_lock;
 * - клиент регистрирует аргументы для передачи в сервер;
 * - клиент выполняет запрос функцией #urpc_client_exec;
 * - клиент считывает результаты выполнения;
 * - клиент разблокирует канал передачи данных функцией #urpc_client_unlock.
 *
 * Регистрация и считывание данных для обмена между сервером и клиентом производится
 * при помощи объекта \link uRpcData \endlink. Указатель на этот объект возвращает
 * функция #urpc_client_lock в случае успеха.
 *
 * Клиент вызывает функции сервера используя функцию #urpc_client_exec, в которую передает
 * идентификатор этой функции.
 *
 * Функция #urpc_client_exec возвращает один из статусов выполнения запроса:
 *
 * - #URPC_STATUS_OK - запрос успешно выполнен;
 * - #URPC_STATUS_FAIL - общая ошибка ывполнения запроса;
 * - #URPC_STATUS_TIMEOUT - превышено время ожидание ответа;
 * - #URPC_STATUS_TRANSPORT_ERROR - ошибка при передаче данных;
 * - #URPC_STATUS_VERSION_MISMATCH - не совпадают версии протоколов;
 * - #URPC_STATUS_TOO_MANY_CONNECTIONS - число уже подключенных клиентов больше установленного сервером ограничения;
 * - #URPC_STATUS_AUTH_ERROR - ошибка при проверке аутентификации.
 *
 * Если RPC запрос успешно выполнен возвращается значение #URPC_STATUS_OK, но это относится
 * только к механизму RPC. Успешность выполнения самой функции на сервере необходимо
 * передавать отдельно через данные объекта \link uRpcData \endlink.
 *
 * В случае ошибки при обмене данными клиент переходит в отключенное состояние, кроме случая
 * использования UDP и ошибки #URPC_STATUS_TIMEOUT. Дальнейшее использование объекта в этом
 * случае невозможно.
 *
 * После завершения работы с RPC сервером необходимо отключиться от сервера и удалить объект
 * RPC клиента функцией #urpc_client_destroy.
 *
*/

#ifndef _urpc_client_h
#define _urpc_client_h

#include <urpc-exports.h>
#include <urpc-types.h>
#include <urpc-data.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct uRpcClient uRpcClient;


/*! Создание RPC клиента.
 *
 * Создаёт RPC клиент для связи с сервером заданым адресом uri. Адрес задается в виде строки:
 * "<type>://name:port", где:
 * - &lt;type&gt; - тип RPC ( udp, tcp, shm );
 * - name - имя или ip адрес системы;
 * - port - номер udp или tcp порта.
 *
 * Для IP версии 6 ip адрес должен быть задан в прямых скобках [], например [::1/128].
 * Для shm номер порта может быть любым или отсутствовать.
 *
 * \param uri адрес сервера;
 * \param max_data_size размер буфера приема-передачи в байтах;
 * \param timeout максимальное время выполнения запроса в секундах.
 *
 * \return Указатель на uRpcClient объект в случае успеха, иначе NULL.
 *
*/
URPC_EXPORT uRpcClient *urpc_client_create( const char *uri, uint32_t max_data_size, double timeout );


/*! Удаление RPC клиента.
 *
 * Закрывает соединение с сервером и удаляет RPC клиент.
 *
 * \param urpc_client указатель на uRpcClient объект.
 *
 * \return Нет.
 *
*/
URPC_EXPORT void urpc_client_destroy( uRpcClient *urpc_client );


/*! Задание механизма безопасности.
 *
 * Определяет механизм безопасности используемый для взаимодействия с сервером. По
 * умолчанию для взаимодействия с сервером не используется никаких механизмов аутентификации
 * и шифрования.
 *
 * \param urpc_client указатель на uRpcClient объект;
 * \param mode режим безопасности.
 *
 * \return 0 если режим безопасности успешно установлен, отрицательное число в случае ошибки.
 *
*/
URPC_EXPORT int urpc_client_set_security( uRpcClient *urpc_client, uRpcSecurity mode );


/*! Задание клиентского ключа аутентификации.
 *
 * Задаёт ключ используемый для аутентификации клиента на сервере.
 *
 * \param urpc_client указатель на uRpcClient объект;
 * \param priv_key указатель на секретный клиентский ключ.
 *
 * \return 0 если ключ успешно задан, отрицательное число в случае ошибки.
 *
*/
URPC_EXPORT int urpc_client_set_client_key( uRpcClient *urpc_client, const unsigned char *priv_key );


/*! Задание серверного ключа аутентификации.
 *
 * Задаёт ключ используемый для аутентификации сервера клиентом.
 *
 * \param urpc_client указатель на uRpcClient объект;
 * \param pub_key указатель на публичный серверный ключ.
 *
 * \return 0 если ключ успешно задан, отрицательное число в случае ошибки.
 *
*/
URPC_EXPORT int urpc_client_set_server_key( uRpcClient *urpc_client, const unsigned char *pub_key );


/*! Подключение к серверу.
 *
 * Производит подключение к серверу с использованием выбранного механизма безопасности.
 *
 * \param urpc_client указатель на uRpcClient объект.
 *
 * \return 0 если подключение к серверу установлено, отрицательное число в случае ошибки.
 *
*/
URPC_EXPORT int urpc_client_connect( uRpcClient *urpc_client );


/*! Блокировка канала передачи.
 *
 * Ждет пока освободится транспортный уровень для передачи данных, обнуляет буфер передаваемых
 * данных и блокирует канал передачи для текущего вызывающего потока.
 *
 * \param urpc_client указатель на uRpcClient объект.
 *
 * \return Указатель на объект \link uRpcData \endlink в случае успешного завершения, иначе NULL.
 *
*/
URPC_EXPORT uRpcData *urpc_client_lock( uRpcClient *urpc_client );


/*! Вызов удалённой процедуры.
 *
 * Производит передачу параметров процедуры, её вызов и передачу результата работы. Успешное
 * завершение функции говорит только о том, что удалённая процедура была вызвана и результат работы
 * получен обратно. Возвращаемое значение удалённой процедуры должно передаваться среди результатов.
 *
 * \param urpc_client указатель на uRpcClient объект;
 * \param proc_id идентификатор вызываемой процедуры.
 *
 * \return Статус выполнения запроса.
 *
*/
URPC_EXPORT uint32_t urpc_client_exec( uRpcClient *urpc_client, uint32_t proc_id );


/*! Освобождение канала передачи.
 *
 * Освобождает канал передачи делая его доступным другим потокам.
 *
 * \param urpc_client указатель на uRpcClient объект.
 *
 * \return Нет.
 *
*/
URPC_EXPORT void urpc_client_unlock( uRpcClient *urpc_client );


#ifdef __cplusplus
} // extern "C"
#endif

#endif // _urpc_client_h
