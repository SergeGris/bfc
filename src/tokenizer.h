/*  tokenizer.h
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

#ifndef _TOKENIZER_H
#define _TOKENIZER_H 1

#include <stddef.h>
#include <limits.h>

#include "system.h"

/*
 * For optimized result, BrainFuck code is first converted to token-value pairs:
 * increment, amount
 * pointer increment, amount
 * label, index
 * jump to label, index
 * read input
 * print output
 */
typedef enum
{
  T_COMMENT = 0,
  T_INCDEC,
  T_POINTER_INCDEC,
  T_LABEL,
  T_JUMP,
  T_GETCHAR,
  T_PUTCHAR,
  T_MAX
} Token;

/* Single Brainfuck command after parsing.  */
typedef struct
{
  Token token;
  i32 value;
} Command;

/* Complete Brainfuck program after parsing and optimizing.  */
typedef struct
{
  Command *tokens;
  size_t length;
  bool have_getchar_commands;
  bool have_putchar_commands;
} ProgramSource;

extern int tokenize (const char *const source,
                     const size_t source_len,
                     Command **out_result,
                     size_t *out_result_len)
  __nonnull ((1, 3, 4));

verify (CHAR_BIT == 8 && T_COMMENT == 0);

static const Token token_table[0400] =
{
  ['+'] = T_INCDEC,
  ['-'] = T_INCDEC,
  ['>'] = T_POINTER_INCDEC,
  ['<'] = T_POINTER_INCDEC,
  ['['] = T_LABEL,
  [']'] = T_JUMP,
  [','] = T_GETCHAR,
  ['.'] = T_PUTCHAR
};
#define parse_token(symbol) (token_table[(unsigned char) symbol])

static const Token token_value_table[0400] =
{
  ['+'] = +1,
  ['-'] = -1,
  ['>'] = +1,
  ['<'] = -1
};
#define parse_value(symbol) (token_value_table[(unsigned char) symbol])

#endif /* _TOKENIZER_H */
