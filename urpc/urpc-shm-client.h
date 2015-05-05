/*
 * uRpc - rpc (remote procedure call) library.
 *
 * Copyright 2009, 2010, 2014, 2015 Andrei Fadeev
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

/* Заголовочный файл клиента удалённых вызовов процедур через механизм разделяемой памяти и семафоры.
 * Функции клиента используются библиотекой uRPC самостоятельно и не предназначены для пользователей. */

#ifndef _urpc_shm_client_h
#define _urpc_shm_client_h

#include <urpc-types.h>
#include <urpc-data.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct uRpcSHMClient uRpcSHMClient;


/* Функция создаёт RPC клиента и подключается к серверу по протоколу SHM.
 * Параметры функции аналогичны urpc_server_create. */
uRpcSHMClient *urpc_shm_client_create( const char *uri );


/* Функция удаляет клиента. */
void urpc_shm_client_destroy( uRpcSHMClient *urpc_udp_client );


/* Функция блокирует канал связи. */
uRpcData *urpc_shm_client_lock( uRpcSHMClient *urpc_udp_client );


/* Функция производит отправку запроса серверу и приём от него ответа. */
uint32_t urpc_shm_client_exchange( uRpcSHMClient *urpc_udp_client );


/* Функция разблокирует канал связи. */
void urpc_shm_client_unlock( uRpcSHMClient *urpc_udp_client );


#ifdef __cplusplus
} // extern "C"
#endif

#endif // _urpc_shm_client_h
