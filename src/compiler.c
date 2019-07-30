/*  compiler.c
    Copyright (C) 2019 Sergey Sushilin
    This file is part of the BrainFuck Compiler

    BrainFuck Compiler is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation, either version 3 of
    the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#include <config.h>

#include "compiler.h"

#include <assert.h>
#include <error.h>
#include <stdio.h>
#include <stdlib.h>

#include "system.h"

#include "arch.h"

#include "xalloc.h"

int
str_append (char **str, const char *format, ...)
{
  /* This is only used to combine arguments,
     so fixed-size string should be safe to use.  */
  char formatted_str[512];
  assert (strlen (format) <= sizeof (formatted_str));

  va_list argp;
  va_start (argp, format);

  vsprintf (formatted_str, format, argp);

  const size_t old_length = (*str == NULL || **str == '\0' ? 0 : strlen (*str));
  char *new_str = xcalloc (old_length + strlen (formatted_str) + 1, sizeof (char));

  if (*str != NULL)
    strcat (new_str, *str);
  strcat (new_str, formatted_str);

  free (*str);
  *str = new_str;

  return 0;
}

static int
write_file (const char *filename,
            const char *source,
            const size_t source_length)
{
  FILE *fp = fopen (filename, "w");
  if (unlikely (fp == NULL))
    {
      error (0, errno, "%s", filename);
      return -1;
    }
  size_t result = fwrite (source, sizeof (char), source_length, fp);
  if (result != source_length)
    {
      error (0, errno, "%s", filename);
      return -1;
    }
  fclose (fp);
  return 0;
}

int
translate_to_asm (const char *filename,
                  ProgramSource *const source)
{
  char *instructions = NULL;
  size_t instructions_length = 0;
  int err = tokens_to_asm (source, &instructions, &instructions_length);
  if (err == 0)
    {
      err = write_file (filename, instructions, instructions_length);
      free (instructions);
    }
  return err;
}
