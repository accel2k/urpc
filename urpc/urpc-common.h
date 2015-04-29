/*
 * uRpc - rpc (remote procedure call) library.
 *
 * Copyright 2014, 2015 Andrei Fadeev
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

#ifndef _urpc_common_h
#define _urpc_common_h

#include <urpc-types.h>
#include <urpc-exports.h>
#include <urpc-network.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


/*! \brief Все поля RPC заголовка представлены в сетевом (big endian) порядке следования байт. */
#define URPC_MAGIC                         0x75525043  /*!< Идентификатор RPC пакета - строка 'uRPC'. */
#define URPC_VERSION                       0x00030000  /*!< Версия протокола uRPC - старшие 16 бит - MAJOR, младшие 16 бит - MINOR. */

/*! \brief Системные идентификаторы параметров. */
#define URPC_PARAM_PROC                    0x00010000  /*!< Идентификатор вызываемой функции - uint32_t. */
#define URPC_PARAM_STATUS                  0x00020000  /*!< Идентификатор статуса - uint32_t. */
#define URPC_PARAM_CAP                     0x00030000  /*!< Идентификатор возможностей сервера - uint32_t. */

/*! \brief Системные идентификаторы процедур. */
#define URPC_PROC_GET_CAP                  0x00010000  /*!< Получение возможностей сервера. */
#define URPC_PROC_LOGIN                    0x00020000  /*!< Начало сессии. */
#define URPC_PROC_LOGOUT                   0x00030000  /*!< Окончание сессии. */

/*! \brief Системные идентификаторы состояния подключения клиента. */
#define URPC_STATE_CONNECTED               0x00010000  /*!< Подключено. */
#define URPC_STATE_NOT_CONNECTED           0x00020000  /*!< Не подключено. */
#define URPC_STATE_GOT_SESSION_ID          0x00030000  /*!< Получен идентификатор сессии. */


typedef struct uRpcHeader {

  uint32_t          magic;
  uint32_t          version;
  uint32_t          session;
  uint32_t          size;

} uRpcHeader;


URPC_EXPORT uRpcType urpc_get_type( const char *uri );


URPC_EXPORT struct addrinfo *urpc_get_sockaddr( const char *uri );


#ifdef __cplusplus
} // extern "C"
#endif

#endif // _urpc_common_h
