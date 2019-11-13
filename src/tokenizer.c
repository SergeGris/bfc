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

#include "die.h"
#include "xalloc.h"

static void
append_to_array (const Command cmd,
                 Command **out_result,
                 size_t *out_result_len)
{
  (*out_result)[(*out_result_len)++] = cmd;
}

int
tokenize (const char *const source,
          size_t source_len,
          Command **out_result,
          size_t *out_result_len)
{
  /* Count [ and ] commands. Difference should be 0 at the end of the program, so
     that all jumps have a matching label.  */
  size_t opening_label_count = 0;
  size_t closing_label_count = 0;

  /* Initialize final result */
  *out_result = NULL;
  *out_result_len = 0;
  Command *result = xmalloc (source_len * sizeof (*result));
  size_t result_len = 0;

  /* Command that is currently being constructed.  */
  Command command = { T_COMMENT, 0 };

  int errorcode = 0;
  for (size_t i = 0; i < source_len; i++)
    {
      unsigned char c_current = source[i];
      unsigned char c_next = source[i + 1];

      Token current = parse_token (c_current);
      Token next = parse_token (c_next);

      command.token = current;

      /* Set value for this command:
         Data increment and pointer increment are added to previous symbol.
         Labels and jumps need a number.
         Read and print need nothing.  */
      if (current == T_INCDEC || current == T_POINTER_INCDEC)
        command.value += parse_value (c_current);
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

          size_t kk = result_len - 1;
          do
            {
              if (result[kk].token == T_JUMP)
                open_labels_to_skip++;
              else if (result[kk].token == T_LABEL && open_labels_to_skip-- == 0)
                {
                  correct_label = result[kk].value;
                  break;
                }
            }
          while (kk-- != 0);

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
      return errorcode;
    }

  /* Copy allocated final result to the arguments.  */
  *out_result = result;
  *out_result_len = result_len;

  return 0;
}
