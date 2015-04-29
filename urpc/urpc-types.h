/*
 * uRpc - rpc (remote procedure call) library.
 *
 * Copyright 2015 Andrei Fadeev
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

#ifndef _urpc_types_h
#define _urpc_types_h


/*! \brief Значения по умолчанию. */
#define URPC_DEFAULT_SERVER_TIMEOUT        2.0
#define URPC_DEFAULT_CLIENT_TIMEOUT        5.0

#define URPC_MAX_DATA_SIZE                 16*1024*1024
#define URPC_DEFAULT_DATA_SIZE             65000
#define URPC_HEADER_SIZE                   sizeof( uRpcHeader )
#define URPC_DEFAULT_BUFFER_SIZE           ( URPC_DEFAULT_DATA_SIZE + URPC_HEADER_SIZE )

#define URPC_MAX_THREADS_NUM               32
#define URPC_MIN_TIMEOUT                   0.1


/*! \brief Пользовательские идентификаторы. */
#define URPC_PARAM_USER                    0x20000000  /*!< Идентификатор начала пользовательских параметров. */
#define URPC_PROC_USER                     0x20000000  /*!< Идентификатор начала пользовательских функций. */


/*! \brief Статус выполнения. */
#define URPC_STATUS_OK                     0x00010000  /*!< Выполнено. */
#define URPC_STATUS_FAIL                   0x00020000  /*!< Общая ошибка. */
#define URPC_STATUS_TIMEOUT                0x00030000  /*!< Превышено время ожидания ответа. */
#define URPC_STATUS_TRANSPORT_ERROR        0x00040000  /*!< Ошибка при передаче данных. */
#define URPC_STATUS_VERSION_MISMATCH       0x00050000  /*!< Не совпадают версии протоколов. */
#define URPC_STATUS_TOO_MANY_CONNECTIONS   0x00060000  /*!< Число уже подключенных клиентов больше установленного сервером ограничения. */
#define URPC_STATUS_AUTH_ERROR             0x00070000  /*!< Ошибка при проверке аутентификации. */


typedef enum { URPC_UNKNOWN = 0, URPC_UDP, URPC_TCP, URPC_SHM } uRpcType;

typedef enum { URPC_SECURITY_NO = 0, URPC_SECURITY_PRIVKEY_AUTH, URPC_SECURITY_PUBKEY_AUTH, URPC_SECURITY_PRIVKEY_ENCRYPT, URPC_SECURITY_PUBKEY_ENCRYPT } uRpcSecurity;


#endif // _urpc_types_h
