/*  tokenizer.c
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

#include "tokenizer.h"

#include <error.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "system.h"

#include "xalloc.h"

Token
parse_token (char symbol)
{
  switch (symbol)
    {
    case '+':
    case '-':
      return T_INCDEC;
    case '<':
    case '>':
      return T_POINTER_INCDEC;
    case '[':
      return T_LABEL;
    case ']':
      return T_JUMP;
    case ',':
      return T_GETCHAR;
    case '.':
      return T_PUTCHAR;
    default:
      return T_COMMENT;
    }
}

int
parse_value (char symbol)
{
  switch (symbol)
    {
    case '+':
    case '>':
      return +1;
    case '-':
    case '<':
      return -1;
    default:
      return 0;
    }
}

char *
strip_comments (const char *const source)
{
  size_t size = strlen (source);
  char *result = xmalloc (size + 1);
  size_t j = 0;
  for (size_t i = 0; i < size; i++)
    if (parse_token (source[i]) != T_COMMENT)
      result[j++] = source[i];
  result[j] = '\0';
  return xrealloc (result, j);
}

static void
append_to_array (const Command cmd,
                 Command **out_result,
                 size_t *out_result_len)
{
  (*out_result)[(*out_result_len)++] = cmd;
}

int
tokenize (const char *const source,
          Command **out_result,
          size_t *out_result_len)
{
  /* Count [ and ] commands. Difference should be 0 at the end of the program, so
     that all jumps have a matching label.  */
  i32 opening_label_count = 0;
  i32 closing_label_count = 0;

  /* Strip comments from the source.  */
  char *cleaned_source = strip_comments (source);

  /* Initialize final result */
  *out_result_len = 0;
  *out_result = NULL;
  size_t result_len = 0;
  Command *result = xmalloc (strlen (cleaned_source) * sizeof (*result));

  /* Command that is currently being constructed.  */
  Command command = { T_COMMENT, 0 };

  int errorcode = 0;
  for (size_t i = 0; i < strlen (cleaned_source); i++)
    {
      char c_current = cleaned_source[i];
      char c_next = cleaned_source[i + 1];

      Token current = parse_token (c_current);
      Token next = parse_token (c_next);

      command.token = current;

      /* Set value for this command:
         Data increment and pointer increment are added to previous symbol.
         Labels and jumps need a number.
         Read and print need nothing.  */
      if (current == T_INCDEC)
        command.value += parse_value (c_current);
      else if (current == T_POINTER_INCDEC)
        {
          /*
          const i32 value = parse_value (c_current);
          command.value += value;
          */
          command.value += parse_value (c_current);
        }
      else if (current == T_LABEL)
        command.value = opening_label_count++;
      else if (current == T_JUMP)
        {
          closing_label_count++;
          if (closing_label_count > opening_label_count)
            {
              /* Error: Label mismatch.  */
              errorcode = 102;
              break;
            }

          /* Traverse final result backwards to find the correct label.  */
          i32 correct_label = -1;
          ptrdiff_t open_labels_to_skip = 0;
          for (size_t kk = result_len - 1;; kk--)
            {
              if (result[kk].token == T_JUMP)
                open_labels_to_skip++;
              else if (result[kk].token == T_LABEL && open_labels_to_skip-- == 0)
                {
                  correct_label = result[kk].value;
                  break;
                }

              if (kk == 0)
                break;
            }
          if (correct_label < 0)
            {
              errorcode = 102;
              error (0, 0, _("label mismatch"));
              break;
            }
          command.value = correct_label;
        }

      /* Expecting new command: Push previous command to the final result and make a new one.  */
      if (current != next || (current != T_INCDEC && current != T_POINTER_INCDEC))
        {
          if (command.token != T_COMMENT)
            append_to_array (command, &result, &result_len);
          command.token = current;
          command.value = 0;
        }
    }
  if (errorcode == 0)
    {
      if (opening_label_count != closing_label_count)
        {
          /* Error: Label mismatch.  */
         errorcode = 102;
         error (0, 0, _("label mismatch"));
        }
    }
  else
    {
      free (result);
      free (cleaned_source);
      return errorcode;
    }

  free (cleaned_source);

  /* Copy allocated final result to the arguments.  */
  *out_result = result;
  *out_result_len = result_len;

  return 0;
}
