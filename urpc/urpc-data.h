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

/*!
 * \file urpc-data.h
 *
 * \author Andrei Fadeev
 * \date 20.02.2014
 * \brief Заголовочный файл библиотеки работы с RPC данными.
 *
 * \defgroup uRpcData uRpcData - библиотека буфера приема-передачи RPC данных.
 *
 * Буфер используется для хранения данных в процессе RPC обмена. Он состоит из
 * двух буферов: приема и передачи. Данные могут быть зарегистрированы в буфере
 * и считаны из него. При регистрации данные автоматически размещаются в
 * буфере передачи, а считываются из буфера приема.
 *
 * Каждый буфер может хранить дополнительный заголовок, в котором система
 * uRpc размещает свои служебные данные.
 *
 * При регистрации данных функцией #urpc_data_set возвращается указатель на область
 * памяти. Пользователь может записывать и считывать данные из этой области в
 * границах заданного размера. Эти данные будут переданы серверу или клиенту в неизменном виде.
 *
 * Указатель на принятые данные можно получить функцией #urpc_data_get.
 *
 * Так как клиент и сервер могут работать на разных архитектурах, включая архитектуры
 * с отличающимся порядком следования байт необходимо учитывать это при разработке. Для
 * упрощения обмена стандартными типами данных реализованы функции urpc_data_set_&lt;type&gt; и
 * urpc_data_get_&lt;type&gt;, где &lt;type&gt; один следуюших типов данных:
 *
 *  - int32 - 32-х битный знаковый целый;
 *  - uint32 - 32-х битный беззнаковый целый;
 *  - int64 - 64-х битный знаковый целый;
 *  - uint64 - 64-х битный беззнаковый целый;
 *  - float - число с плавающей запятой одинарной точности;
 *  - double - число с плавающей запятой двойной точности;
 *  - string - строка с нулем на конце.
 *
 * Для строковых данных дополнительно доступна функция #urpc_data_dup_string которая
 * возвращает указатель на копию строки из буфера. После использования этой строки
 * необходимо освободить память функцией #urpc_data_free_string.
 *
 * При использовании этих функций будет автоматически происходить преобразование данных
 * в зависимости от архитектуры.
 *
 * Создание объекта uRpcData производится функцией #urpc_data_create. Память под буферы
 * может быть выделена пользователем заранее или автоматически выделится объектом.
 *
 * Удаление объекта производится функцией #urpc_data_destroy.
 *
*/

#ifndef _urpc_data_h
#define _urpc_data_h

#include <urpc-exports.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef enum { URPC_DATA_INPUT = 1, URPC_DATA_OUTPUT = 2 } uRpcDataDirection;

typedef struct uRpcData uRpcData;


/*! Создание объекта для работы с RPC данными.
 *
 * При работе с uRpc этот объект автоматически создается клиентом или сервером.
 * Если указатели ibuffer и(или) obuffer равен(ы) NULL память под буфер будет выделена
 * автоматически.
 *
 * Если параметр clean установлен в значение отличное от нуля, при очистке буфера
 * или изменении его размера будет производится очистка неиспользуемой памяти. Это
 * необходимо для исключения возможности чтения данных другим клиентом при повторном
 * использовании буфера.
 *
 * \param buffer_size размер каждого из буферов приема и передачи в байтах;
 * \param header_size размер заголовка в начале каждого буфера в байтах;
 * \param ibuffer указатель на буфер принимаемых данных или NULL;
 * \param obuffer указатель на буфер отправляемых данных или NULL;
 * \param clean признак очистки буфера.
 *
 * \return Указатель на RPC буфер или NULL в случае ошибки.
 *
*/
URPC_EXPORT uRpcData *urpc_data_create( uint32_t buffer_size, uint32_t header_size, void *ibuffer, void *obuffer, int clean );


/*! Удаление объекта.
 *
 * Если память под буферы выделялась автоматически, освободится она тоже автоматически.
 *
 * \param urpc_data указатель на RPC буфер.
 *
 * \return Нет.
 *
*/
URPC_EXPORT void urpc_data_destroy( uRpcData *urpc_data );


/*! Возвращает размер заголовка в начале буфера.
 *
 * \param urpc_data указатель на RPC буфер.
 *
 * \return Размер заголовка в байтах.
 *
*/
URPC_EXPORT uint32_t urpc_data_get_header_size( uRpcData *urpc_data );


/*! Изменяет размер заголовка в начале буфера.
 *
 * \param urpc_data указатель на RPC буфер;
 * \param header_size новый размер заголовка в байтах.
 *
 * \return 0 если размер заголовка изменен, отрицательное число в случае ошибки.
*/
URPC_EXPORT int urpc_data_set_header_size( uRpcData *urpc_data, uint32_t header_size );


/*! Получает указатель на заголовок в начале буфера.
 *
 * \param urpc_data указатель на RPC буфер;
 * \param direction тип буфера принимаемых - URPC_DATA_INPUT или отправляемых - URPC_DATA_OUTPUT данных.
 *
 * \return Указатель на заголовок в начале буфера.
 *
*/
URPC_EXPORT void* urpc_data_get_header( uRpcData *urpc_data, uRpcDataDirection direction );


/*! Записывает данные в заголовок в начале буфера.
 *
 * \param urpc_data указатель на RPC буфер;
 * \param direction тип буфера принимаемых - URPC_DATA_INPUT или отправляемых - URPC_DATA_OUTPUT данных;
 * \param header указатель на данные для записи в заголовок;
 * \param header_size размер данных для записи в заголовок.
 *
 * \return 0 если данные были записаны, отрицательное число в случае ошибки.
 *
*/
URPC_EXPORT int urpc_data_set_header( uRpcData *urpc_data, uRpcDataDirection direction, void *header, uint32_t header_size );


/*! Получает размер который занимают данные в буфере.
 *
 * \param urpc_data указатель на RPC буфер;
 * \param direction тип буфера принимаемых - URPC_DATA_INPUT или отправляемых - URPC_DATA_OUTPUT данных.
 *
 * \return Размер данных в буфере в байтах.
 *
*/
URPC_EXPORT uint32_t urpc_data_get_data_size( uRpcData *urpc_data, uRpcDataDirection direction );


/*! Изменяет размер который занимают данные в буфере.
 *
 * \param urpc_data указатель на RPC буфер;
 * \param direction тип буфера принимаемых - URPC_DATA_INPUT или отправляемых - URPC_DATA_OUTPUT данных;
 * \param data_size новый размер данных в буфере.
 *
 * \return 0 если размер данных изменен, отрицательное число в случае ошибки.
 *
*/
URPC_EXPORT int urpc_data_set_data_size( uRpcData *urpc_data, uRpcDataDirection direction, uint32_t data_size );


/*! Получает указатель на данные в буфере.
 *
 * \param urpc_data указатель на RPC буфер;
 * \param direction тип буфера принимаемых - URPC_DATA_INPUT или отправляемых - URPC_DATA_OUTPUT данных.
 *
 * \return Указатель на данные в буфере.
 *
*/
URPC_EXPORT void* urpc_data_get_data( uRpcData *urpc_data, uRpcDataDirection direction );


/*! Записывает данные в буфер.
 *
 * \param urpc_data указатель на RPC буфер;
 * \param direction тип буфера принимаемых - URPC_DATA_INPUT или отправляемых - URPC_DATA_OUTPUT данных;
 * \param data указатель на данные для записи в буфер;
 * \param data_size размер данных для записи в буфер.
 *
 * \return 0 если данные были записаны, отрицательное число в случае ошибки.
 *
*/
URPC_EXPORT int urpc_data_set_data( uRpcData *urpc_data, uRpcDataDirection direction, void *data, uint32_t data_size );


/*! Проверяет границы данных в буфере.
 *
 * \param urpc_data указатель на RPC буфер;
 * \param direction тип буфера принимаемых - URPC_DATA_INPUT или отправляемых - URPC_DATA_OUTPUT данных.
 *
 * \return 0 если смещения и границы данных не выходят за размер буфера, отрицательное число в случае ошибки.
 *
*/
URPC_EXPORT int urpc_data_validate( uRpcData *urpc_data, uRpcDataDirection direction );


/*! Проверяет зарегистрирована переменная в буфере исходящих данных или нет.
 *
 * \param urpc_data указатель на RPC буфер;
 * \param id идентификатор переменной.
 *
 * \return 1 если переменная с таким идентификатором уже была зарегистрирована, иначе 0.
 *
*/
URPC_EXPORT int urpc_data_is_set( uRpcData *urpc_data, uint32_t id );


/*! Запись значения / регистрация / изменение размера переменной в буфере передачи
 *
 * Записывает значение или регистрирует переменную по ее идентификатору
 * в буфере передачи или изменяет размер уже зарегистрированной переменной.
 *
 * Можно изменить размер только самой последней зарегистрированной переменной и только
 * в сторону уменьшения её размера. При этом object должен равняться NULL.
 *
 * \param urpc_data указатель на RPC буфер;
 * \param id идентификатор переменной;
 * \param object указатель на записываемые данные (может быть NULL);
 * \param size размер данных.
 *
 * \return Адрес переменной в буфере в случае успешного завершения, иначе NULL.
 *
*/
URPC_EXPORT void* urpc_data_set( uRpcData *urpc_data, uint32_t id, const void *object, uint32_t size );


/*! Получение указателя на переменную в буфере приема по идентификатору
 *
 * Возвращает указатель на хранимую переменную и ее размер по идентификатору
 *
 * \param urpc_data указатель на RPC буфер;
 * \param id идентификатор переменной;
 * \param size указатель по которому будет сохранен размер переменной (может быть NULL).
 *
 * \return Указатель на переменную в буфере в случае успешного завершения, иначе NULL.
 *
*/
URPC_EXPORT void* urpc_data_get( uRpcData *urpc_data, uint32_t id, uint32_t *size );


/*! Регистрация переменной типа 32-х битный знаковый целый в буфере передачи.
 *
 * \param urpc_data указатель на RPC буфер;
 * \param id идентификатор переменной;
 * \param value значение переменной.
 *
 * \return 0 в случае успешной регистрации, отрицательное число в случае ошибки.
 *
*/
URPC_EXPORT int urpc_data_set_int32( uRpcData *urpc_data, uint32_t id, int32_t value );


/*! Считывание значения переменной типа 32-х битный знаковый целый из буфера приема.
 *
 * \param urpc_data указатель на RPC буфер;
 * \param id идентификатор переменной.
 *
 * \return Значение переменной. Если переменная не зарегистрирована возвращается 0.
 *
*/
URPC_EXPORT int32_t urpc_data_get_int32( uRpcData *urpc_data, uint32_t id );


/*! Регистрация переменной типа 32-х битный беззнаковый целый в буфере передачи.
 *
 * \param urpc_data указатель на RPC буфер;
 * \param id идентификатор переменной;
 * \param value значение переменной.
 *
 * \return 0 в случае успешной регистрации, отрицательное число в случае ошибки.
 *
*/
URPC_EXPORT int urpc_data_set_uint32( uRpcData *urpc_data, uint32_t id, uint32_t value );


/*! Считывание значения переменной типа 32-х битный беззнаковый целый из буфера приема.
 *
 * \param urpc_data указатель на RPC буфер;
 * \param id идентификатор переменной.
 *
 * \return Значение переменной. Если переменная не зарегистрирована возвращается 0.
 *
*/
URPC_EXPORT uint32_t urpc_data_get_uint32( uRpcData *urpc_data, uint32_t id );


/*! Регистрация переменной типа 64-х битный знаковый целый в буфере передачи.
 *
 * \param urpc_data указатель на RPC буфер;
 * \param id идентификатор переменной;
 * \param value значение переменной.
 *
 * \return 0 в случае успешной регистрации, отрицательное число в случае ошибки.
 *
*/
URPC_EXPORT int urpc_data_set_int64( uRpcData *urpc_data, uint32_t id, int64_t value );


/*! Считывание значения переменной типа 64-х битный знаковый целый из буфера приема.
 *
 * \param urpc_data указатель на RPC буфер;
 * \param id идентификатор переменной.
 *
 * \return Значение переменной. Если переменная не зарегистрирована возвращается 0.
 *
*/
URPC_EXPORT int64_t urpc_data_get_int64( uRpcData *urpc_data, uint32_t id );


/*! Регистрация переменной типа 64-х битный беззнаковый целый в буфере передачи.
 *
 * \param urpc_data указатель на RPC буфер;
 * \param id идентификатор переменной;
 * \param value значение переменной.
 *
 * \return 0 в случае успешной регистрации, отрицательное число в случае ошибки.
 *
*/
URPC_EXPORT int urpc_data_set_uint64( uRpcData *urpc_data, uint32_t id, uint64_t value );


/*! Считывание значения переменной типа 64-х битный беззнаковый целый из буфера приема.
 *
 * \param urpc_data указатель на RPC буфер;
 * \param id идентификатор переменной.
 *
 * \return Значение переменной. Если переменная не зарегистрирована возвращается 0.
 *
*/
URPC_EXPORT uint64_t urpc_data_get_uint64( uRpcData *urpc_data, uint32_t id );


/*! Регистрация переменной с плавающей запятой одинарной точности в буфере передачи.
 *
 * \param urpc_data указатель на RPC буфер;
 * \param id идентификатор переменной;
 * \param value значение переменной.
 *
 * \return 0 в случае успешной регистрации, отрицательное число в случае ошибки.
 *
*/
URPC_EXPORT int urpc_data_set_float( uRpcData *urpc_data, uint32_t id, float value );


/*! Считывание значения переменной с плавающей запятой одинарной точности из буфера приема.
 *
 * \param urpc_data указатель на RPC буфер;
 * \param id идентификатор переменной.
 *
 * \return Значение переменной. Если переменная не зарегистрирована возвращается 0.
 *
*/
URPC_EXPORT float urpc_data_get_float( uRpcData *urpc_data, uint32_t id );


/*! Регистрация переменной с плавающей запятой двойной точности в буфере передачи.
 *
 * \param urpc_data указатель на RPC буфер;
 * \param id идентификатор переменной;
 * \param value значение переменной.
 *
 * \return 0 в случае успешной регистрации, отрицательное число в случае ошибки.
 *
*/
URPC_EXPORT int urpc_data_set_double( uRpcData *urpc_data, uint32_t id, double value );


/*! Считывание значения переменной с плавающей запятой двойной точности из буфера приема.
 *
 * \param urpc_data указатель на RPC буфер;
 * \param id идентификатор переменной.
 *
 * \return Значение переменной. Если переменная не зарегистрирована возвращается 0.
 *
*/
URPC_EXPORT double urpc_data_get_double( uRpcData *urpc_data, uint32_t id );


/*! Регистрация строки с нулем на конце в буфере передачи.
 *
 * \param urpc_data указатель на RPC буфер;
 * \param id идентификатор переменной;
 * \param string строка для регистрации.
 *
 * \return 0 в случае успешной регистрации, отрицательное число в случае ошибки.
 *
*/
URPC_EXPORT int urpc_data_set_string( uRpcData *urpc_data, uint32_t id, const char *string );


/*! Считывание значения строки из буфера приема.
 *
 *  Функция возвращает указатель на данные в буфере. Этот указатель действителен только
 *  во время блокировки канала передачи. Его содержимое доступно только для чтения.
 *  При необходимости пользователь может получить копию этой строки функцией #urpc_data_dup_string.
 *
 * \param urpc_data указатель на RPC буфер;
 * \param id идентификатор переменной.
 *
 * \return Указатель на строку из буфера или NULL если переменная не зарегистрирована.
 *
*/
URPC_EXPORT const char* urpc_data_get_string( uRpcData *urpc_data, uint32_t id );


/*! Считывание значения строки из буфера приема.
 *
 *  Функция возвращает указатель на копию строки с данными из буфера.
 *  Пользователь должен самостоятельно освободить память функцией #urpc_data_free_string.
 *
 * \param urpc_data указатель на RPC буфер;
 * \param id идентификатор переменной.
 *
 * \return Указатель на новую строку или NULL если переменная не зарегистрирована.
 *
*/
URPC_EXPORT char* urpc_data_dup_string( uRpcData *urpc_data, uint32_t id );


/*! Освобождение памяти выделенной под строку.
 *
 * \param string указатель на строку.
 *
 * \return Нет.
 *
*/
URPC_EXPORT void urpc_data_free_string( char *string );


#ifdef __cplusplus
} // extern "C"
#endif

#endif // _urpc_data_h
