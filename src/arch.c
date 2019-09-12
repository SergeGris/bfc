/*  arch.c
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

#include "arch.h"

#include <assert.h>
#include <error.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "system.h"

#include "compiler.h"
#include "die.h"

int
exec (char **arg)
{
  int status = -1;
  pid_t pid  = fork ();

  if (pid < 0)
    /* Fork error.  */
    die (EXIT_FAILURE, errno, "%s", arg[0]);
  else if (pid == 0)
    {
      /* Child process.  */
      execvp (arg[0], arg);
      die (EXIT_FAILURE, errno, "%s", arg[0]);
    }
  else /* pid > 0 */
    /* Parent process.  */
    wait (&status);

  return status;
}

#if defined(__x86_64__)
# include "x86_64.c"
#elif defined(__i386__)
# include "i386.c"
#else
# error Unknown arch
#endif

int
tokens_to_asm (ProgramSource *const source,
               char **final_output,
               size_t *final_output_length)
{
  *final_output = NULL;
  *final_output_length = 0;
  char *output = NULL;

  str_append (&output, init_variables, DATA_ARRAY_SIZE);
  str_append (&output, init_section_text);

  /* Subroutines for I/O.  */
  if (source->have_getchar_commands)
    str_append (&output, getchar_body);
  if (source->have_putchar_commands)
    str_append (&output, putchar_body);

  /* Execution starts at this point.  */
  str_append (&output, start_begin_and_init_array);

  /* Convert tokens to machine code.  */
  int errorcode = 0;
  for (size_t i = 0; i < source->length && errorcode == 0; i++)
    {
      const Command current = source->tokens[i];
      switch (current.token)
        {
        case T_INCDEC:
          if (current.value > 0)
            str_append (&output, increment_current_value, +current.value & 0xFF);
          else if (current.value < 0)
            str_append (&output, decrement_current_value, -current.value & 0xFF);
          else
            {
              /* Command has no effect.  */
              ;
            }
          break;

        case T_POINTER_INCDEC:
          if (current.value > 0)
            str_append (&output, increment_current_pointer, +current.value);
          else if (current.value < 0)
            str_append (&output, decrement_current_pointer, -current.value);
          else
            {
              /* Command has no effect.  */
              ;
            }
          break;

        case T_LABEL:
          str_append (&output, label_begin, current.value, current.value);
          break;
        case T_JUMP:
          str_append (&output, label_end, current.value, current.value);
          break;

        case T_GETCHAR:
          if (!source->have_getchar_commands)
            /* Error: Unexpected token.  */
            errorcode = 202;
          else
            str_append (&output, call_getchar);
          break;

        case T_PUTCHAR:
          if (!source->have_putchar_commands)
            /* Error: Unexpected token.  */
            errorcode = 202;
          else
            str_append (&output, call_putchar);
          break;

        case T_COMMENT:
        default:
          break;
        }
    }

  /* Write quit commands.  */
  if (errorcode == 0)
    str_append (&output, start_end);
  else
    {
      free (output);
      return errorcode;
    }

  *final_output = output;
  *final_output_length = strlen (output);

  return 0;
}
