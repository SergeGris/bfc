/*  optimizer.c
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

#include "optimizer.h"

#include <error.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "system.h"

#include "xalloc.h"

int
optimize (const Command *const tokens,
          const size_t tokens_len,
          ProgramSource *out_result,
          const unsigned int level)
{
  int err = 0;

  Command *input_tokens = xmalloc (tokens_len * sizeof (Command));
  size_t input_len = tokens_len;
  memcpy (input_tokens, tokens, tokens_len * sizeof (Command));

  bool have_putchar_commands = true,
       have_getchar_commands = true;

  /* Level 0:
     Return code as is.

     Level 1:
     Remove inactive loops (no +-, before the loop start)
     Check if there are no output commands
     Check if there are no input  commands.  */
  if (level >= 1)
    {
      /* Remove inactive loops.  */
      const int max_passes = 10;
      bool finished = false;
      int inactive_loop_index = -1;
      for (int round = 0; round < max_passes && !finished; round++)
        {
          for (size_t i = 0; i < input_len; i++)
            {
              const Command current = input_tokens[i];
              if (current.token == T_INCDEC || current.token == T_GETCHAR)
                {
                  /* Not inactive */
                  finished = true;
                  break;
                }
              else if (current.token == T_LABEL)
                {
                  inactive_loop_index = current.value;
                  break;
                }
            }
          bool inside_loop = false;
          for (size_t i = 0; i < input_len; i++)
            {
              const Command current = input_tokens[i];
              if (current.token == T_LABEL && current.value == inactive_loop_index)
                inside_loop = true;
              if (inside_loop)
                {
                  input_tokens[i].token = T_COMMENT;
                  if (current.token == T_JUMP && current.value == inactive_loop_index)
                    break;
                }
            }
        }

      /* Find input and print commands.  */
      bool found_getchar = false,
           found_putchar = false;
      for (size_t i = 0; i < input_len; i++)
        {
          const Command current = input_tokens[i];

          if (current.token == T_GETCHAR)
            found_getchar = true;
          else if (current.token == T_PUTCHAR)
            found_putchar = true;

          if (found_getchar && found_putchar)
            break;
        }

      have_getchar_commands = found_getchar;
      have_putchar_commands = found_putchar;
    }

  /* TODO: Level 2:
     If no print or input commands, program has no effect
     => Either remove everything or replace with [] if infinite loop.  */
  if (level >= 2)
    {
      /* Error: Not implemented.  */
      err = 103;
      error (0, 0, _("optimization level %i is not implemented"), level);

      if (!have_putchar_commands && !have_getchar_commands)
        { }
    }

  if (err != 0)
    {
      free (input_tokens);
      return err;
    }

  size_t input_len_without_comments = input_len;
  for (size_t i = 0; i < input_len; i++)
    if (input_tokens[i].token == T_COMMENT)
      input_len_without_comments--;

  /* Compact result by stripping out comments.  */
  out_result->tokens = xmalloc (input_len_without_comments * sizeof (Command));

  size_t length = 0;
  for (size_t i = 0; i < input_len; i++)
    {
      if (input_tokens[i].token != T_COMMENT)
        out_result->tokens[length++] = input_tokens[i];
    }

  out_result->length = length;
  out_result->have_putchar_commands = have_putchar_commands;
  out_result->have_getchar_commands = have_getchar_commands;

  free (input_tokens);

  return 0;
}

int
tokenize_and_optimize (const char *const source,
                       ProgramSource *out_result,
                       const unsigned int level)
{
  out_result->tokens = NULL;
  out_result->length = 0;

  Command *tokenized_source;
  size_t tokenized_source_length = 0;
  int err = tokenize (source, &tokenized_source, &tokenized_source_length);
  if (err != 0)
    return err;

  err = optimize (tokenized_source, tokenized_source_length, out_result, level);
  free (tokenized_source);

  return err;
}
