/*
 * uRPC - rpc (remote procedure call) library.
 *
 * Copyright 2014-2015 Andrei Fadeev (andrei@webcontrol.ru)
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ERROR_CODE -1

#define BUFFER_SIZE   32*1024*1024
#define HEADER_SIZE   16
#define MAX_PARAMS    1024

#if defined _MSVC_COMPILER
#define snprintf sprintf_s
#endif

int
main (int    argc,
      char **argv)
{
  int do_export = 0;
  int do_import = 0;
  int show_help = 0;

  uRpcData *urpc_data;

  void *data;
  uint32_t data_size;
  void *file_data;
  uint32_t file_size;

  uint32_t iparam;
  uint32_t iparams[MAX_PARAMS];
  float fparam;
  float fparams[MAX_PARAMS];
  double dparam;
  double dparams[MAX_PARAMS];
  const char *sparam;
  char *sparams[MAX_PARAMS];
  unsigned int sparam_length;
  char **strings;

  FILE *fio;
  char *fio_name = NULL;

  int i, j;

  if (argc == 1)
    {
      show_help = 1;
    }
  else
    {
      for (i = 1; i < argc - 1; i++)
        {
          if ((strcmp (argv[i], "-e") == 0) || strcmp (argv[i], "--export") == 0)
            do_export = 1;
          else if ((strcmp (argv[i], "-i") == 0) || strcmp (argv[i], "--import") == 0)
            do_import = 1;
          else if ((strcmp (argv[i], "-h") == 0) || strcmp (argv[i], "--help") == 0)
            show_help = 1;
          else
            fprintf (stderr, "%s: unknown option '%s' in command line\n", argv[0], argv[i]);
        }
      fio_name = argv[argc - 1];
      if (fio_name[0] == '-')
        show_help = 1;
    }

  if ((!do_export && !do_import) || (show_help) || (fio_name == NULL))
    {
      fprintf (stderr, "\nUsage:\n");
      fprintf (stderr, "  %s: [OPTION...] FILE\n\n", argv[0]);
      fprintf (stderr, "Options:\n");
      fprintf (stderr, "  -e, --export     Perform data export\n");
      fprintf (stderr, "  -i, --import     Perform data export\n");
      fprintf (stderr, "\n\n");
      return show_help ? 0 : -1;
    }

  urpc_data = urpc_data_create (BUFFER_SIZE, HEADER_SIZE, NULL, NULL, 1);

  /* Подготавливаем набор тестовых данных. */
  strings = malloc (MAX_PARAMS * sizeof (char*));
  for (i = 0; i < MAX_PARAMS; i++)
    {
      iparams[i] = 251 * i;
      fparams[i] = (float) (2.0 * ((257.0 * i) / (257.0 * (MAX_PARAMS - 1)) - 0.5));
      dparams[i] = 2.0 * ((263.0 * i) / (263.0 * (MAX_PARAMS - 1)) - 0.5);

      sparams[i] = malloc (128);
      snprintf (sparams[i], 128, "Test string %10u, %+8.6f, %+8.6lf", iparams[i], fparams[i], dparams[i]);

      if (i == 0 || i == MAX_PARAMS - 1)
        {
          strings[i] = NULL;
        }
      else
        {
          strings[i] = malloc (128);
          snprintf (strings[i], 128, "Test string %d", i);
        }
    }

  /* Размещаем тестовые данные в буффере. */
  if (do_export)
    {
      for (i = 0; i < MAX_PARAMS; i++)
        {
          strings[0] = sparams[i];
          sparam_length = (int)strlen (sparams[i]) + 1;
          urpc_data_set_strings (urpc_data, 5 * i, strings);
          urpc_data_set_uint32 (urpc_data, 5 * i + 1, sparam_length);
          urpc_data_set_uint32 (urpc_data, 5 * i + 2, iparams[i]);
          urpc_data_set_float (urpc_data, 5 * i + 3, fparams[i]);
          urpc_data_set_double (urpc_data, 5 * i + 4, dparams[i]);
        }

      data = urpc_data_get_data (urpc_data, URPC_DATA_OUTPUT);
      data_size = urpc_data_get_data_size (urpc_data, URPC_DATA_OUTPUT);

      fio = fopen (fio_name, "wb");
      fwrite (data, data_size, 1, fio);
      fclose (fio);
    }

  /* Очищаем буффер с данными. */
  urpc_data_set_data_size (urpc_data, URPC_DATA_OUTPUT, 0);
  urpc_data_set_data_size (urpc_data, URPC_DATA_INPUT, 0);

  /* Считываем и проверяем тестовые данные. */
  if (do_import)
    {
      fio = fopen (fio_name, "rb");
      fseek (fio, 0, SEEK_END);
      file_size = ftell (fio);
      fseek (fio, 0, SEEK_SET);
      file_data = malloc (file_size);
      fread (file_data, file_size, 1, fio);
      fclose (fio);

      urpc_data_set_data (urpc_data, URPC_DATA_INPUT, file_data, file_size);

      free (file_data);

      for (i = 0; i < MAX_PARAMS; i++)
        {
          if (urpc_data_get_strings_length (urpc_data, 5 * i) != MAX_PARAMS - 1)
            {
            printf ("Parameter %d number mismatch\n", i);
            exit (ERROR_CODE);
            }
          for (j = 1; j < MAX_PARAMS - 1; j++)
            {
              char *string = urpc_data_dup_string (urpc_data, 5 * i, j);
              if (strcmp (string, strings[j]) != 0)
                {
                printf ("Parameter %d, %d string mismatch\n", i, j);
                exit (ERROR_CODE);
                }
              urpc_data_free_string (string);
            }
          sparam = urpc_data_get_string (urpc_data, 5 * i, 0);
          urpc_data_get_uint32 (urpc_data, 5 * i + 1, &iparam);

          sparam_length = strlen (sparam) + 1;
          if (sparam_length != iparam)
            {
            printf ("Parameter %d size mismatch %u != %u\n", i, sparam_length, iparam);
            exit (ERROR_CODE);
            }
          if (strcmp (sparam, sparams[i]) != 0)
            {
            printf ("Parameter %d string mismatch\n", i);
            exit (ERROR_CODE);
            }

          urpc_data_get_uint32 (urpc_data, 5 * i + 2, &iparam);
          if (iparams[i] != iparam)
            {
            printf ("Parameter %d int mismatch\n", i);
            exit (ERROR_CODE);
            }

          urpc_data_get_float (urpc_data, 5 * i + 3, &fparam);
          if (fparams[i] != fparam)
            {
            printf ("Parameter %d float mismatch\n", i);
            exit (ERROR_CODE);
            }

          urpc_data_get_double (urpc_data, 5 * i + 4, &dparam);
          if (dparams[i] != dparam)
            {
            printf ("Parameter %d double mismatch\n", i);
            exit (ERROR_CODE);
            }
        }
    }

  urpc_data_destroy (urpc_data);

  printf ("All done\n");

  return 0;
}
