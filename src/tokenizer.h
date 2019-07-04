
#ifndef _TOKENIZER_H
#define _TOKENIZER_H

#include <stdbool.h>
#include <stddef.h>

/*
 * For optimized result, Brainfuck code is first converted to token-value pairs:
 * increment, amount
 * pointer increment, amount
 * label, index
 * jump to label, index
 * read input
 * print output
 */
typedef enum
{
  T_COMMENT,
  T_INC,
  T_POINTER_INC,
  T_LABEL,
  T_JUMP,
  T_INPUT,
  T_PRINT
} Token;

/* Single Brainfuck command after parsing.  */
typedef struct
{
  Token token;
  int value;
} Command;

/* Complete Brainfuck program after parsing and optimizing.  */
typedef struct
{
  Command *tokens;
  size_t length;
  bool no_print_commands;
  bool no_input_commands;
} ProgramSource;

Token parse_token (const char symbol);
int parse_value (const char symbol);
char *strip_comments (const char *const source);

int tokenize (const char *const source, Command ** out_result,
	      size_t *out_result_len);
int optimize (const Command * const tokens, const size_t tokens_len,
	      ProgramSource * out_result, const int level);
int tokenize_and_optimize (const char *const source,
			   ProgramSource * out_result, const int level);

#endif /* _TOKENIZER_H */
