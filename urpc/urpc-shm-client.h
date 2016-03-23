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

/* Заголовочный файл клиента удалённых вызовов процедур через механизм разделяемой
   памяти и семафоры. Функции клиента используются библиотекой uRPC самостоятельно
   и не предназначены для пользователей. */

#ifndef __URPC_SHM_CLIENT_H__
#define __URPC_SHM_CLIENT_H__

#include <urpc-types.h>
#include <urpc-data.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _uRpcSHMClient uRpcSHMClient;

/* Функция создаёт RPC клиента и подключается к серверу по протоколу SHM.
   Параметры функции аналогичны urpc_server_create. */
uRpcSHMClient *urpc_shm_client_create                  (const char            *uri);

/* Функция удаляет клиента. */
void           urpc_shm_client_destroy                 (uRpcSHMClient         *urpc_shm_client);

/* Функция блокирует канал связи. */
uRpcData      *urpc_shm_client_lock                    (uRpcSHMClient         *urpc_shm_client);

/* Функция производит отправку запроса серверу и приём от него ответа. */
uint32_t       urpc_shm_client_exchange                (uRpcSHMClient         *urpc_shm_client);

/* Функция разблокирует канал связи. */
void           urpc_shm_client_unlock                  (uRpcSHMClient         *urpc_shm_client);

/* Функция возвращает указатель на строку с локальным адресом в формате URI. */
const char    *urpc_shm_client_get_self_address        (uRpcSHMClient         *urpc_shm_client);

/* Функция возвращает указатель на строку с адресом сервера в формате URI. */
const char    *urpc_shm_client_get_peer_address        (uRpcSHMClient         *urpc_shm_client);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* __URPC_SHM_CLIENT_H__ */
