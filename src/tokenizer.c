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

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "system.h"

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
    case ';':
      return T_GETNUMBER;
    case ':':
      return T_PUTNUMBER;
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
  const size_t size = strlen (source);
  char *result = xmalloc (size + 1);
  size_t j = 0;
  for (size_t i = 0; i < size; ++i)
    if (parse_token (source[i]) != T_COMMENT)
      result[j++] = source[i];
  result[j] = '\0';
  return result;
}

static int
append_to_array (const Command cmd,
                 Command **out_result,
                 size_t *out_result_len)
{
  size_t new_size = *out_result_len + 1;
  Command *tmp = xrealloc (*out_result, new_size * sizeof (Command));
  tmp[new_size - 1] = cmd;

  *out_result = tmp;
  *out_result_len = new_size;
  return 0;
}

int
tokenize (const char *const source,
          Command **out_result,
          size_t *out_result_len)
{
  /* Count [ and ] commands. Difference should be 0 at the end of the program, so
     that all jumps have a matching label.  */
  int opening_label_count = 0;
  int closing_label_count = 0;

  /* Initialize final result */
  *out_result_len = 0;
  *out_result = NULL;
  size_t result_len = 0;
  Command *result = NULL;

  /* Strip comments from the source.  */
  char *cleaned_source = strip_comments (source);

  /* Command that is currently being constructed.  */
  Command command = { T_COMMENT, 0 };

  int errorcode = 0;
  for (size_t i = 0; i < strlen (cleaned_source); ++i)
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
          const int32_t value = parse_value (c_current);
          command.value += value;
        }
      else if (current == T_LABEL)
        command.value = opening_label_count++;
      else if (current == T_JUMP)
        {
          ++closing_label_count;
          if (closing_label_count > opening_label_count)
            {
              /* Error: Label mismatch.  */
              errorcode = 102;
              break;
            }

          /* Traverse final result backwards to find the correct label.  */
          int correct_label = -1;
          int open_labels_to_skip = 0;
          for (size_t kk = result_len - 1;; --kk)
            {
              if (result[kk].token == T_JUMP)
                ++open_labels_to_skip;
              else if (result[kk].token == T_LABEL)
                {
                  if (open_labels_to_skip-- == 0)
                    {
                      correct_label = result[kk].value;
                      break;
                    }
                }

              if (kk == 0)
                break;
            }
          if (correct_label < 0)
            {
              /* Error: Label mismatch */
              errorcode = 102;
              break;
            }
          command.value = correct_label;
        }

      /* Expecting new command: Push previous command to the final result and make a new one.  */
      if (current != next || (current != T_INCDEC && current != T_POINTER_INCDEC))
        {
          if (command.token != T_COMMENT)
            {
              int err = append_to_array (command, &result, &result_len);
              if (err != 0)
                {
                  /* Error: Out of memory.  */
                  errorcode = 101;
                  break;
                }
            }
          command.token = current;
          command.value = 0;
        }
    }
  if (errorcode == 0)
    {
      if (opening_label_count != closing_label_count)
        /* Error: Label mismatch.  */
        errorcode = 102;
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

int
optimize (const Command *const tokens,
          const size_t tokens_len,
          ProgramSource *out_result,
          const int level)
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
      for (int round = 0; round < max_passes && !finished; ++round)
        {
          for (size_t i = 0; i < input_len; ++i)
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
          for (size_t i = 0; i < input_len; ++i)
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
      bool found_input  = false,
           found_output = false;
      for (size_t i = 0; i < input_len; ++i)
        {
          const Command current = input_tokens[i];

          if (current.token == T_GETCHAR)
            found_input = true;
          else if (current.token == T_PUTCHAR)
            found_output = true;

          #if !STRICT_MULLER

          #endif

          if (found_input && found_output)
            break;
        }

      have_putchar_commands = found_output;
      have_getchar_commands = found_input;
    }

  /* TODO: Level 2:
     If no print or input commands, program has no effect
     => Either remove everything or replace with [] if infinite loop.  */
  if (level >= 2)
    {
      /* Error: Not implemented.  */
      err = 103;
      error (0, 0, _("optimization level %d is not implemented"), level);

      if (!have_putchar_commands && !have_getchar_commands)
        { }
    }

  if (err != 0)
    {
      free (input_tokens);
      return err;
    }

  size_t input_len_without_comments = input_len;
  for (size_t i = 0; i < input_len; ++i)
    if (input_tokens[i].token == T_COMMENT)
      --input_len_without_comments;

  /* Compact result by stripping out comments.  */
  out_result->tokens = xmalloc (input_len_without_comments * sizeof (Command));

  size_t length = 0;
  for (size_t i = 0; i < input_len; ++i)
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
                       const int level)
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
