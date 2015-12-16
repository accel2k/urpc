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
 * \file urpc-network.h
 *
 * \brief Заголовочный файл библиотеки совместимости с BSD socket
 * \author Andrei Fadeev (andrei@webcontrol.ru)
 * \date 2009-2015
 * \license GNU General Public License version 3 или более поздняя<br>
 * Коммерческая лицензия - свяжитесь с автором
 *
 * \defgroup uRpcNetwork uRpcNetwork - библиотека совместимости с BSD socket.
 *
 * Библиотека предназначена для кроссплатформенной работы с сетевыми функциями BSD socket.
 * Windows содержит практически все необходимые функции стека BSD socket с некоторыми
 * отличиями которые устраняет данная библиотека.
 *
 * При создании сокета в Windows возвращается тип SOCKET, в то время как в Unix системах
 * используется тип int. Признаком ошибки в Unix системах является число -1, в Windows
 * используется константа INVALID_SOCKET. Библиотека urpc-network определяет две эти
 * константы для Unix систем. Помимо этого определяются две функции closesocket и ioctlsocket
 * являющиеся полными аналогами функция close и ioctl при работе с сокетами.
 *
 * В Windows перед использованием сетевых функций необходимо выполнить инициализацию сетевой
 * подсистемы. В Unix подобной инициализации проводить не требуется, но для унификации кода
 * определены две функции #urpc_network_init и #urpc_network_close которые должны вызываться
 * в самом начале программы и в её конце.
 *
 * Для обработки ошибок сетевой подсистемы определены функции #urpc_network_last_error и
 * #urpc_network_last_error_str и две наиболее часто используемые константы EAGAIN и EINTR.
 * Также определена константа MSG_NOSIGNAL.
 *
 * Библиотека содержит три дополнительные функции:
 *
 * - #urpc_network_set_tcp_nodelay - отключение алгоритма Нейгла;
 * - #urpc_network_set_reuse - разрешение использования адреса уже использовавшегося ранее;
 * - #urpc_network_set_non_block - перевод соединения в неблокирующий режим.
 *
 * Практически все основные функции BSD socket можно использовать без изменений, включая: socket, bind,
 * connect, accept, recv, send, select и др.
 *
 */

#ifndef __URPC_NETWORK_H__
#define __URPC_NETWORK_H__

#include <urpc-exports.h>

#if defined(_WIN32)

#include <winsock2.h>
#include <ws2tcpip.h>

#define URPC_MSG_NOSIGNAL      0
#define URPC_EAGAIN            WSAEWOULDBLOCK
#define URPC_EINTR             WSAEINTR

#endif

#if defined(__unix__)

#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>

#define SOCKET                 int
#define INVALID_SOCKET         -1
#define closesocket            close
#define ioctlsocket            ioctl

#define URPC_MSG_NOSIGNAL      0
#define URPC_EAGAIN            EAGAIN
#define URPC_EINTR             EINTR

#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 *
 * Функция инициализирует сетевую подсистему. Функция инициализации вызывается в самом начале
 * програмы. Функция может быть вызвана несколько раз, в этом случае функция завершения работы
 * должна быть вызвана такое же число раз. В Unix является заглушкой.
 *
 * \return 0 - в случае успеха, иначе -1.
 *
 */
URPC_EXPORT
int            urpc_network_init               (void);

/**
 *
 * Функция завершает работу с сетевой подсситемой. Функция вызывается в конце работы программы.
 * Должна быть вызвана* такое же число раз как #urpc_network_init. В Unix является заглушкой.
 *
 * \return Нет.
 *
 */
URPC_EXPORT
void           urpc_network_close              (void);

/**
 *
 * Функция возвращает код последней ошибки. В Unix возвращается значение errno, в Windows
 * значение функции WSAGetLastError.
 *
 * \return Код последней ошибки.
 *
 */
URPC_EXPORT
int            urpc_network_last_error         (void);

/**
 *
 * Функция возвращает строку с описанием последней ошибки. Возвращаемая строка не должна
 * модифицироваться вызывающей програмой.
 *
 * \return Строка с описанием последней ошибки.
 *
 */
URPC_EXPORT
const char    *urpc_network_last_error_str     (void);

/**
 *
 * Функция отключает объединение маленьких пакетов в один для TCP соединений (алгоритм Нейгла)
 * и соответственно уменьшает задержки в отправке данных.
 *
 * \param socket дескриптор сокета.
 *
 * \return 0 - в случае успеха, иначе -1.
 *
 */
URPC_EXPORT
int            urpc_network_set_tcp_nodelay    (SOCKET                 socket);

/**
 *
 * Функция разрешает использовать IP адрес до момента полного завершения предыдуших соединений.
 * Становится возможным повторный запуск программы сразу после её завершений.
 *
 * \param socket дескриптор сокета.
 *
 * \return 0 - в случае успеха, иначе -1.
 *
 */
URPC_EXPORT
int            urpc_network_set_reuse          (SOCKET                 socket);

/**
 *
 * Функция переводит соединение в неблокирующий режим. После перевода в неблокирующий режим
 * функции recv и send будут немедленно завершать свою работу в независимости от размера
 * принятых и переданных данных. Для реализации полноценного обмена становится необходимым
 * использование функции select.
 *
 * \param socket дескриптор сокета.
 *
 * \return 0 - в случае успеха, иначе -1.
 *
*/
URPC_EXPORT
int            urpc_network_set_non_block      (SOCKET                 socket);

#ifdef __cplusplus
}
#endif

#endif /* __URPC_NETWORK_H__ */
