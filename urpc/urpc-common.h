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

#ifndef _urpc_common_h
#define _urpc_common_h

#include <urpc-types.h>
#include <urpc-exports.h>
#include <urpc-network.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


/*  Максимальная длина адреса RPC сервера/клиента. */
#define MAX_HOST_LEN                       1024

/*  Максимальная длина строки с номером TCP/UDP порта. */
#define MAX_PORT_LEN                       5

/* Размер заголовка uRPC пакета. */
#define URPC_HEADER_SIZE                   sizeof( uRpcHeader )

/* Размер буфера данных по умолчанию (максимальный для UDP). */
#define URPC_DEFAULT_BUFFER_SIZE           ( URPC_DEFAULT_DATA_SIZE + URPC_HEADER_SIZE )

/* Минимально возможный таймаут процедуры обмена данными. */
#define URPC_MIN_TIMEOUT                   0.1


/* Все поля RPC заголовка представлены в сетевом (big endian) порядке следования байт. */
#define URPC_MAGIC                         0x75525043  /* Идентификатор RPC пакета - строка 'uRPC'. */
#define URPC_VERSION                       0x00030000  /* Версия протокола uRPC - старшие 16 бит - MAJOR, младшие 16 бит - MINOR. */


/* Системные идентификаторы параметров. */
#define URPC_PARAM_PROC                    0x00010000  /* Идентификатор вызываемой функции - uint32_t. */
#define URPC_PARAM_STATUS                  0x00020000  /* Идентификатор статуса - uint32_t. */
#define URPC_PARAM_CAP                     0x00030000  /* Идентификатор возможностей сервера - uint32_t. */


/* Системные идентификаторы процедур. */
#define URPC_PROC_GET_CAP                  0x00010000  /* Получение возможностей сервера. */
#define URPC_PROC_LOGIN                    0x00020000  /* Начало сессии. */
#define URPC_PROC_LOGOUT                   0x00030000  /* Окончание сессии. */


/* Системные идентификаторы состояния подключения клиента. */
#define URPC_STATE_CONNECTED               0x00010000  /* Подключено. */
#define URPC_STATE_NOT_CONNECTED           0x00020000  /* Не подключено. */
#define URPC_STATE_GOT_SESSION_ID          0x00030000  /* Получен идентификатор сессии. */


/* Структура заголовка uRPC пакета. */
typedef struct uRpcHeader {

  uint32_t          magic;                             /* Идентификатор пакета uRPC. */
  uint32_t          version;                           /* Версия протокола uRPC. */
  uint32_t          session;                           /* Идентификатор сессии клиента. */
  uint32_t          size;                              /* Размер пакета. */

} uRpcHeader;


/* Структура управляющего сегмента общей области памяти. */
typedef struct uRpcSHMControl {

  uint32_t          pid;                               /* Идентификатор процесса сервера. */
  uint32_t          size;                              /* Размер буфера RPC данных. */
  uint32_t          threads_num;                       /* Число потоков сервера (буферов RPC данных). */

} uRpcSHMControl;


/* Функция возвращает тип протокола передачи данных для указанного RPC адреса. */
URPC_EXPORT uRpcType urpc_get_type( const char *uri );


/* Функция возвращает указатель на структуру addrinfo с информацией о сетевых адресах.
 * Формат сетевого адреса аналогичен возвращаемому функцией getaddrinfo. */
URPC_EXPORT struct addrinfo *urpc_get_sockaddr( const char *uri );


#ifdef __cplusplus
} // extern "C"
#endif

#endif // _urpc_common_h
