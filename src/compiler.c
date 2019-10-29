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
str_append (char **str, size_t *length, const char *format, ...)
{
  /* This is only used to combine arguments,
     so fixed-size string should be safe to use.  */
  char formatted_str[384];
  assert (strlen (format) <= sizeof (formatted_str));

  va_list argp;
  va_start (argp, format);
  int err = vsprintf (formatted_str, format, argp);
  va_end (argp);

  size_t formatted_str_len = strlen (formatted_str);
  *str = xrealloc (*str, *length + formatted_str_len + 1);
  strcpy (*str + *length, formatted_str);
  *length += formatted_str_len;

  return err >= 0 ? 0 : -1;
}

static int
write_file (const char *filename,
            const char *source,
            const size_t source_length)
{
  FILE *fp;
  if ((fp = fopen (filename, "w")) != NULL
      && fwrite (source, sizeof (char), source_length, fp) == source_length
      && fclose (fp) == 0 ? true : (fp = NULL, false))
    return 0;

  error (0, errno, "%s", quotef (filename));
  if (fp != NULL && fclose (fp) != 0)
    error (0, errno, "%s", quotef (filename));

  return -1;
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
