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

#include "urpc-data.h"
#include "urpc-endian.h"

#include <string.h>
#include <stdlib.h>

#define URPC_DATA_TYPE   0x54445275

#define DATA_ALIGN_SIZE  sizeof (uint32_t)     /* Минимальный размер переменной. */

typedef struct
{
  uint8_t             *data;                   /* Указатель на данные в буфере приемо-передачи. */
  uint32_t             buffer_size;            /* Размер буфера. */
  uint32_t             data_size;              /* Размер данных. */
} DataBuffer;

typedef struct
{
  uint32_t             id;                     /* Идентификатор переменной. */
  uint32_t             size;                   /* Размер переменной. */
  uint32_t             next;                   /* Смещение от начала текущего заголовка до следующего,
                                                  если == 0 - это последняя переменная. */
  uint8_t              data[DATA_ALIGN_SIZE];  /* Данные, реальный размер содержится в size,
                                                  но не менее DATA_ALIGN_SIZE байт. */
} DataParam;

struct _uRpcData
{
  uint32_t             urpc_data_type;         /* Тип объекта uRpcData. */

  int                  clean;                  /* Очищать память после использования. */

  uint32_t             buffer_size;            /* Размер всего буфера. */
  uint32_t             header_size;            /* Размер заголовка в буфере перед данными. */
  uint8_t             *ibuffer;                /* Буфер входящих данных. */
  uint8_t             *obuffer;                /* Буфер исходящих данных. */

  int                  ibuffer_created;        /* Память под буфер входящих данных была выделена объектом. */
  int                  obuffer_created;        /* Память под буфер исходящих данных была выделена объектом. */

  DataBuffer           input;
  DataBuffer           output;
};

static DataParam *
urpc_data_find_param (DataBuffer *buffer,
                      uint32_t    id)
{
  DataParam *param = (DataParam *)buffer->data;
  uint32_t left_size = buffer->data_size;
  uint32_t offset = 0;
  uint32_t cur_param_size;

  if (buffer->data_size == 0)
    return NULL;

  while (1)
    {

      uint32_t param_id;
      uint32_t param_size;
      uint32_t param_next;

      /* Проверяем, что в буфере присутствуют данные как минимум размера структуры RpcParam. */
      if (left_size < sizeof (DataParam) - DATA_ALIGN_SIZE)
        return NULL;

      param_id = UINT32_FROM_BE (param->id);
      param_size = UINT32_FROM_BE (param->size);
      param_next = UINT32_FROM_BE (param->next);

      /* Вычисляем размер занимаемый текущим проверяемым параметром и сравниваем
         его с размером данных в буфере. */
      cur_param_size = param_size + sizeof (DataParam) - DATA_ALIGN_SIZE;
      if (cur_param_size > left_size)
        return NULL;

      /* Если идентификаторы совпали, возвращаем указатель на искомый параметр.
         Последний параметр в списке можно определить по нулевому смещению до
         следующего параметра. В этом случае завершаем поиск. */
      if (param_id == id)
        return param;
      if (param_next == 0)
        break;

      /* "Уменьшаем" размер данных в буфере на размер текущего параметра,
         переходим к следующему параметру и повторяем проверки. */
      left_size -= param_next;
      offset += param_next;
      param = (DataParam *) (buffer->data + offset);

    }

  /* Если искомый параметр не был найден возвращается указатель на последний
     параметр в буфере.*/
  return param;
}

static void *
urpc_data_set_param (DataBuffer *buffer,
                     uint32_t    id,
                     const void *object,
                     uint32_t    size)
{
  uint32_t param_id;
  uint32_t param_size;
  uint32_t param_next;

  DataParam *param = urpc_data_find_param (buffer, id);

  if (param != NULL)
    {
      param_id = UINT32_FROM_BE (param->id);
      param_size = UINT32_FROM_BE (param->size);
      param_next = UINT32_FROM_BE (param->next);
    }

  /* Если set_param вызван для последнего зарегистрированного параметра, а так-же
     если object == NULL, то изменим размер этого параметра. Дополнительно проверяется
     наличие достаточного свободного места в буфере при увеличении размера параметра.*/
  if (param != NULL && param_id == id && param_next == 0 && object == NULL)
    {
      if ((size < param_size) || ((buffer->buffer_size - buffer->data_size) >= (size - param_size)))
        {
          buffer->data_size = buffer->data_size - param_size + size;
          param->size = UINT32_TO_BE (size);
          return param->data;
        }
    }

  /* Параметр с таким идентификатором уже есть. */
  if (param != NULL && param_id == id)
    {
      /* Если размер совпадает, установим значение. */
      if (param_size == size)
        {
          memcpy (param->data, object, size);
          return param->data;
        }
      /* Иначе вернем ошибку. */
      else
        {
          return NULL;
        }
    }

  /* Проверка оставшегося места в буфере. */
  if ((buffer->buffer_size - buffer->data_size) < (size + sizeof (DataParam)))
    return NULL;

  /* Смещение до следующего параметра с выравниванием. */
  if (param != NULL)
    {
      uint32_t data_pad = (DATA_ALIGN_SIZE - (param_size % DATA_ALIGN_SIZE));
      data_pad = (data_pad == DATA_ALIGN_SIZE) ? 0 : data_pad;
      param->next = UINT32_TO_BE (param_size + sizeof (DataParam) - DATA_ALIGN_SIZE + data_pad);
      memset (buffer->data + buffer->data_size, 0, data_pad);
      buffer->data_size += data_pad;
    }

  /* Запоминаем параметр в буфере. */
  param = (DataParam *) (buffer->data + buffer->data_size);
  buffer->data_size += size + sizeof (DataParam) - DATA_ALIGN_SIZE;
  param->id = UINT32_TO_BE (id);
  param->size = UINT32_TO_BE (size);
  param->next = 0;

  if (object != NULL)
    memcpy (param->data, object, size);

  return param->data;
}

static void *
urpc_data_get_param (DataBuffer *buffer,
                     uint32_t    id,
                     uint32_t   *size)
{
  DataParam *param = urpc_data_find_param (buffer, id);

  /* Буфер пуст или параметр не найден. */
  if (param == NULL || UINT32_FROM_BE (param->id) != id)
    return NULL;

  if (size != NULL)
    *size = UINT32_FROM_BE (param->size);

  return param->data;
}

uRpcData *
urpc_data_create (uint32_t buffer_size,
                  uint32_t header_size,
                  void    *ibuffer,
                  void    *obuffer,
                  int      clean)
{
  uRpcData *urpc_data;

  if (header_size >= buffer_size)
    return NULL;

  urpc_data = malloc (sizeof (uRpcData));
  if (urpc_data == NULL)
    return NULL;

  if (ibuffer == NULL)
    {
      ibuffer = malloc (buffer_size);
      urpc_data->ibuffer_created = 1;
    }
  else
    {
      urpc_data->ibuffer_created = 0;
    }

  if (obuffer == NULL)
    {
      obuffer = malloc (buffer_size);
      urpc_data->obuffer_created = 1;
    }
  else
    {
      urpc_data->obuffer_created = 0;
    }

  if (ibuffer == NULL || obuffer == NULL)
    {
      if (ibuffer != NULL && urpc_data->ibuffer_created)
        free (ibuffer);
      if (obuffer != NULL && urpc_data->obuffer_created)
        free (obuffer);
      free (urpc_data);
      return NULL;
    }

  urpc_data->urpc_data_type = URPC_DATA_TYPE;
  urpc_data->clean = clean == 0 ? 0 : 1;
  urpc_data->buffer_size = buffer_size;
  urpc_data->header_size = header_size;
  urpc_data->ibuffer = (uint8_t *) ibuffer;
  urpc_data->obuffer = (uint8_t *) obuffer;

  urpc_data->input.data = urpc_data->ibuffer + urpc_data->header_size;
  urpc_data->input.data_size = 0;
  urpc_data->input.buffer_size = urpc_data->buffer_size - urpc_data->header_size;

  urpc_data->output.data = urpc_data->obuffer + urpc_data->header_size;
  urpc_data->output.data_size = 0;
  urpc_data->output.buffer_size = urpc_data->buffer_size - urpc_data->header_size;

  return urpc_data;
}


void
urpc_data_destroy (uRpcData *urpc_data)
{
  if (urpc_data->urpc_data_type != URPC_DATA_TYPE)
    return;

  if (urpc_data->ibuffer_created)
    free (urpc_data->ibuffer);
  if (urpc_data->obuffer_created)
    free (urpc_data->obuffer);
  free (urpc_data);
}

void *
urpc_data_get_header (uRpcData         *urpc_data,
                      uRpcDataDirection direction)
{
  if (urpc_data->urpc_data_type != URPC_DATA_TYPE)
    return NULL;

  if (direction == URPC_DATA_INPUT)
    return urpc_data->ibuffer;
  else if (direction == URPC_DATA_OUTPUT)
    return urpc_data->obuffer;

  return NULL;
}

uint32_t
urpc_data_get_data_size (uRpcData         *urpc_data,
                         uRpcDataDirection direction)
{
  if (urpc_data->urpc_data_type != URPC_DATA_TYPE)
    return 0;

  if (direction == URPC_DATA_INPUT)
    return urpc_data->input.data_size;
  else if (direction == URPC_DATA_OUTPUT)
    return urpc_data->output.data_size;

  return 0;
}

int
urpc_data_set_data_size (uRpcData         *urpc_data,
                         uRpcDataDirection direction,
                         uint32_t          data_size)
{
  DataBuffer *data_buffer;

  if (urpc_data->urpc_data_type != URPC_DATA_TYPE)
    return -1;

  if (direction == URPC_DATA_INPUT)
    data_buffer = &urpc_data->input;
  else if (direction == URPC_DATA_OUTPUT)
    data_buffer = &urpc_data->output;
  else
    return -1;

  if (data_buffer->buffer_size < data_size)
    return -1;

  if (data_buffer->data_size > data_size && urpc_data->clean)
    memset (data_buffer->data + data_size, 0, data_buffer->data_size - data_size);

  data_buffer->data_size = data_size;

  return 0;
}

void *
urpc_data_get_data (uRpcData         *urpc_data,
                    uRpcDataDirection direction)
{
  if (urpc_data->urpc_data_type != URPC_DATA_TYPE)
    return NULL;

  if (direction == URPC_DATA_INPUT)
    return urpc_data->input.data;
  else if (direction == URPC_DATA_OUTPUT)
    return urpc_data->output.data;

  return NULL;
}

int
urpc_data_set_data (uRpcData         *urpc_data,
                    uRpcDataDirection direction,
                    void             *data,
                    uint32_t          data_size)
{
  DataBuffer *data_buffer;

  if (urpc_data->urpc_data_type != URPC_DATA_TYPE)
    return -1;

  if (direction == URPC_DATA_INPUT)
    data_buffer = &urpc_data->input;
  else if (direction == URPC_DATA_OUTPUT)
    data_buffer = &urpc_data->output;
  else
    return -1;

  if (data_buffer->buffer_size < data_size)
    return -1;

  memcpy (data_buffer->data, data, data_size);

  if (data_buffer->data_size > data_size && urpc_data->clean)
    memset (data_buffer->data + data_size, 0, data_buffer->data_size - data_size);

  data_buffer->data_size = data_size;

  return 0;
}

int
urpc_data_validate (uRpcData         *urpc_data,
                    uRpcDataDirection direction)
{
  DataBuffer *buffer;
  DataParam *param;
  uint32_t left_size;
  uint32_t offset = 0;
  uint32_t cur_param_size;

  if (urpc_data->urpc_data_type != URPC_DATA_TYPE)
    return -1;

  if (direction == URPC_DATA_INPUT)
    buffer = &urpc_data->input;
  else if (direction == URPC_DATA_OUTPUT)
    buffer = &urpc_data->output;
  else
    return -1;

  param = (DataParam *) buffer->data;
  left_size = buffer->data_size;

  if (buffer->data_size == 0)
    return 0;

  while (1)
    {
      uint32_t param_size;
      uint32_t param_next;

      /* Проверяем, что в буфере присутствуют данные как минимум размера структуры RpcParam. */
      if (left_size < sizeof (DataParam))
        return -1;

      param_size = UINT32_FROM_BE (param->size);
      param_next = UINT32_FROM_BE (param->next);

      /* Вычисляем размер занимаемый текущим проверяемым параметром и сравниваем
         его с размером данных в буфере. */
      cur_param_size = param_size + sizeof (DataParam) - DATA_ALIGN_SIZE;
      if (cur_param_size > left_size)
        return -1;

      /* Последний параметр в списке можно определить по нулевому смещению до
         следующего параметра. В этом случае завершаем поиск. */
      if (param_next == 0)
        break;

      /* "Уменьшаем" размер данных в буфере на размер текущего параметра,
         переходим к следующему параметру и повторяем проверки.*/
      left_size -= param_next;
      offset += param_next;
      param = (DataParam *) (buffer->data + offset);
    }

  /* Прошлись по всем параметрам, ошибок нет. */
  return 0;
}

int
urpc_data_is_set (uRpcData *urpc_data,
                  uint32_t  id)
{
  DataParam *param;

  if (urpc_data->urpc_data_type != URPC_DATA_TYPE)
    return 0;

  param = urpc_data_find_param (&urpc_data->output, id);
  if (param == NULL || UINT32_FROM_BE (param->id) != id)
    return 0;

  return 1;
}

void *
urpc_data_set (uRpcData   *urpc_data,
               uint32_t    id,
               const void *object,
               uint32_t    size)
{
  if (urpc_data->urpc_data_type != URPC_DATA_TYPE)
    return NULL;

  return urpc_data_set_param (&urpc_data->output, id, object, size);
}

void *
urpc_data_get (uRpcData *urpc_data,
               uint32_t  id,
               uint32_t *size)
{

  if (urpc_data->urpc_data_type != URPC_DATA_TYPE)
    return NULL;

  return urpc_data_get_param (&urpc_data->input, id, size);

}

int
urpc_data_set_int32 (uRpcData *urpc_data,
                     uint32_t  id,
                     int32_t   value)
{
  if (urpc_data->urpc_data_type != URPC_DATA_TYPE)
    return -1;

  value = INT32_TO_BE (value);
  return urpc_data_set_param (&urpc_data->output, id, &value, sizeof (int32_t)) == NULL ? -1 : 0;
}

int
urpc_data_get_int32 (uRpcData *urpc_data,
                     uint32_t  id,
                     int32_t  *value)
{
  int32_t *value_addr;
  uint32_t value_size;

  if (urpc_data->urpc_data_type != URPC_DATA_TYPE)
    return -1;

  value_addr = (int32_t *) urpc_data_get_param (&urpc_data->input, id, &value_size);
  if (value_addr == NULL || value_size != sizeof (int32_t))
    return -1;

  *value = INT32_FROM_BE (*value_addr);
  return 0;
}

int
urpc_data_set_uint32 (uRpcData *urpc_data,
                      uint32_t  id,
                      uint32_t  value)
{
  if (urpc_data->urpc_data_type != URPC_DATA_TYPE)
    return -1;

  value = UINT32_TO_BE (value);
  return urpc_data_set_param (&urpc_data->output, id, &value, sizeof (int32_t)) == NULL ? -1 : 0;
}

int
urpc_data_get_uint32 (uRpcData *urpc_data,
                      uint32_t  id,
                      uint32_t *value)
{
  uint32_t *value_addr;
  uint32_t value_size;

  if (urpc_data->urpc_data_type != URPC_DATA_TYPE)
    return -1;

  value_addr = (uint32_t *) urpc_data_get_param (&urpc_data->input, id, &value_size);
  if (value_addr == NULL || value_size != sizeof (uint32_t))
    return -1;

  *value = UINT32_FROM_BE (*value_addr);
  return 0;
}

int
urpc_data_set_int64 (uRpcData *urpc_data,
                     uint32_t  id,
                     int64_t   value)
{
  if (urpc_data->urpc_data_type != URPC_DATA_TYPE)
    return -1;

  value = INT64_TO_BE (value);
  return urpc_data_set_param (&urpc_data->output, id, &value, sizeof (int64_t)) == NULL ? -1 : 0;
}

int
urpc_data_get_int64 (uRpcData *urpc_data,
                     uint32_t  id,
                     int64_t  *value)
{
  int64_t *value_addr;
  uint32_t value_size;

  if (urpc_data->urpc_data_type != URPC_DATA_TYPE)
    return -1;

  value_addr = (int64_t *) urpc_data_get_param (&urpc_data->input, id, &value_size);
  if (value_addr == NULL || value_size != sizeof (int64_t))
    return -1;

  *value = INT64_FROM_BE (*value_addr);
  return 0;
}

int
urpc_data_set_uint64 (uRpcData *urpc_data,
                      uint32_t  id,
                      uint64_t  value)
{
  if (urpc_data->urpc_data_type != URPC_DATA_TYPE)
    return -1;

  value = UINT64_TO_BE (value);
  return urpc_data_set_param (&urpc_data->output, id, &value, sizeof (uint64_t)) == NULL ? -1 : 0;
}

int
urpc_data_get_uint64 (uRpcData *urpc_data,
                      uint32_t  id,
                      uint64_t *value)
{
  uint64_t *value_addr;
  uint32_t value_size;

  if (urpc_data->urpc_data_type != URPC_DATA_TYPE)
    return -1;

  value_addr = (uint64_t *) urpc_data_get_param (&urpc_data->input, id, &value_size);
  if (value_addr == NULL || value_size != sizeof (uint64_t))
    return -1;

  *value = UINT64_FROM_BE (*value_addr);
  return 0;
}

int
urpc_data_set_float (uRpcData *urpc_data,
                     uint32_t  id,
                     float     value)
{
  void *pvalue = &value;
  uint32_t uvalue = *(uint32_t *) pvalue;

  return urpc_data_set_uint32 (urpc_data, id, uvalue);
}

int
urpc_data_get_float (uRpcData *urpc_data,
                     uint32_t  id,
                     float    *value)
{
  uint32_t uvalue;
  void *pvalue = &uvalue;

  if (urpc_data_get_uint32 (urpc_data, id, &uvalue) != 0)
    return -1;

  *value = *(float *) pvalue;
  return 0;
}

int
urpc_data_set_double (uRpcData *urpc_data,
                      uint32_t  id,
                      double    value)
{
  void *pvalue = &value;
  uint64_t uvalue = *(uint64_t *) pvalue;

  return urpc_data_set_uint64 (urpc_data, id, uvalue);
}

int
urpc_data_get_double (uRpcData *urpc_data,
                      uint32_t  id,
                      double   *value)
{
  uint64_t uvalue;
  void *pvalue = &uvalue;

  if (urpc_data_get_uint64 (urpc_data, id, &uvalue) != 0)
    return -1;

  *value = *(double *) pvalue;
  return 0;
}

int
urpc_data_set_string (uRpcData   *urpc_data,
                      uint32_t    id,
                      const char *string)
{
  size_t length;

  if (urpc_data->urpc_data_type != URPC_DATA_TYPE)
    return -1;

  length = strlen (string) + 1;
  if (length > (urpc_data->output.buffer_size - urpc_data->output.data_size))
    return -1;

  return urpc_data_set_param (&urpc_data->output, id, string, (uint32_t) length) == NULL ? -1 : 0;
}

int
urpc_data_set_strings (uRpcData     *urpc_data,
                       uint32_t      id,
                       char * const *strings)
{
  size_t size;
  char *buffer;
  int i;

  if (urpc_data->urpc_data_type != URPC_DATA_TYPE)
    return -1;

  size = 0;
  for (i = 0; strings[i] != NULL; i++)
      size += strlen (strings[i]) + 1;

  if (size > (urpc_data->output.buffer_size - urpc_data->output.data_size))
    return -1;

  buffer = urpc_data_set_param (&urpc_data->output, id, NULL, (uint32_t) size);
  if (buffer == NULL)
    return -1;

  for (i = 0; strings[i] != NULL; i++)
    {
      size = strlen (strings[i]) + 1;
      memcpy (buffer, strings[i], size);
      buffer += size;
    }

  return 0;
}

const char *
urpc_data_get_string (uRpcData *urpc_data,
                      uint32_t  id,
                      uint32_t  index)
{
  const char *buffer;
  size_t offset;
  uint32_t size;
  uint32_t i;

  if (urpc_data->urpc_data_type != URPC_DATA_TYPE)
    return NULL;

  buffer = (const char *) urpc_data_get_param (&urpc_data->input, id, &size);
  if (buffer == NULL)
    return NULL;

  i = 0;
  offset = 0;
  do
    {
      if (index == i)
        return buffer + offset;
      offset += strlen (buffer + offset) + 1;
      i += 1;
    }
  while (offset < size);

  return NULL;
}

char *
urpc_data_dup_string (uRpcData *urpc_data,
                      uint32_t  id,
                      uint32_t  index)
{
  uint32_t size;
  const char *string;
  char *dup_string;

  string = urpc_data_get_string (urpc_data, id, index);
  if (string == NULL)
    return NULL;

  size = (int)strlen (string) + 1;
  dup_string = malloc (size);
  if (dup_string != NULL)
    memcpy (dup_string, string, size);

  return dup_string;
}

uint32_t
urpc_data_get_strings_length (uRpcData *urpc_data,
                              uint32_t  id)
{
  uint32_t size;
  const char *buffer;
  size_t offset;
  int i;

  if (urpc_data->urpc_data_type != URPC_DATA_TYPE)
    return 0;

  buffer = (const char *) urpc_data_get_param (&urpc_data->input, id, &size);
  if (buffer == NULL)
    return 0;

  i = 0;
  offset = 0;
  do
    {
      offset += strlen (buffer + offset) + 1;
      i += 1;
    }
  while (offset < size);

  return i;
}

void
urpc_data_free_string (char *string)
{
  free (string);
}
