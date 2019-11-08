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
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "system.h"

#include "arch.h"
#include "die.h"
#include "xalloc.h"

void
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

  if (err < 0)
    die (EXIT_FAILURE, errno, "vsprintf()");

  size_t formatted_str_len = strlen (formatted_str);
  *str = xrealloc (*str, *length + formatted_str_len + 1);
  strcpy (*str + *length, formatted_str);
  *length += formatted_str_len;
}

static int
write_file (const char *filename,
            const char *source,
            const size_t source_length)
{
  int fd;
  if ((fd = open (filename, O_WRONLY)) >= 0)
    {
      char const *begin = source;
      char const *const end = begin + source_length;

      while (begin < end)
        {
          size_t remaining = end - begin;
          ssize_t res = write (fd, begin, remaining);

          if (res >= 0)
            {
              begin += res;
              continue; /* Let's send the remaining part of this message.  */
            }
          if (errno == EINTR)
            {
              /* It is just a signal, try again.  */
              continue;
            }
          else
            {
              /* It is a real error.  */
              error (0, errno, "%s", quotef (filename));
              return -1;
            }
        }

      if (close (fd) == 0)
        return 0;
    }

  error (0, errno, "%s", quotef (filename));
  return -1;
}

int
translate_to_asm (const char *filename,
                  ProgramSource *const source)
{
  char *instructions = NULL;
  size_t instructions_length = 0;
  tokens_to_asm (source, &instructions, &instructions_length);
  int err = write_file (filename, instructions, instructions_length);
  free (instructions);
  return err;
}
