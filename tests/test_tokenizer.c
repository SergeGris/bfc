#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <string.h>
#include <stdlib.h>

#include "tokenizer.h"

void test_symbol_parsing(void **state)
{
	assert_true(parse_token('+') == T_INC);
	assert_true(parse_token('-') == T_INC);
	assert_true(parse_token('<') == T_POINTER_INC);
	assert_true(parse_token('>') == T_POINTER_INC);
	assert_true(parse_token('[') == T_LABEL);
	assert_true(parse_token(']') == T_JUMP);
	assert_true(parse_token(',') == T_INPUT);
	assert_true(parse_token('.') == T_PRINT);

	// Unknown symbols should be comments
	assert_true(parse_token('a') == T_COMMENT);
	assert_true(parse_token('b') == T_COMMENT);

	// Test only tokens that should have a value
	// For other tokens, return value does not matter
	assert_true(parse_value('+') == 1);
	assert_true(parse_value('-') == -1);
	assert_true(parse_value('>') == 1);
	assert_true(parse_value('<') == -1);
}

void test_strip_comments(void **state)
{
	// No comments
	{
		const char* input = "++++-[<.,>]";
		const char* correct = input;
		char* result = strip_comments(input);

		assert_true(strcmp(result, correct) == 0);
		free(result);
	}

	// Comments only
	{
		const char* input = "abffd87%g";
		const char* correct = "";
		char* result = strip_comments(input);

		assert_true(strcmp(result, correct) == 0);
		free(result);
	}

	// Both
	{
		const char* input = "dfd54#d+-[]dasd.>,[78]<";
		const char* correct = "+-[].>,[]<";
		char* result = strip_comments(input);

		assert_true(strcmp(result, correct) == 0);
		free(result);
	}
}

void test_tokenize(void **state)
{
	// Correct code
	{
		const char* input = "[][.>[,+[-<]]-]";
		const Token correct_token[] = {T_LABEL, T_JUMP, T_LABEL, T_PRINT, T_POINTER_INC, T_LABEL,
			T_INPUT, T_INC, T_LABEL, T_INC, T_POINTER_INC, T_JUMP, T_JUMP, T_INC, T_JUMP};

		Command *output;
		unsigned int output_length = 0;
		int err = tokenize(input, &output, &output_length);

		assert_true(err == 0);
		assert_true(output_length == strlen(input));

		// Tokens
		for (unsigned int i = 0; i < output_length; i++) {
			assert_true(output[i].token == correct_token[i]);
		}
		// Label addresses
		assert_true(output[0].value == output[1].value);
		assert_true(output[2].value == output[14].value);
		assert_true(output[5].value == output[12].value);
		assert_true(output[8].value == output[11].value);

		// Increment directions
		assert_true(output[4].value == 1);
		assert_true(output[7].value == 1);
		assert_true(output[9].value == -1);
		assert_true(output[10].value == -1);
		assert_true(output[13].value == -1);

		free(output);
	}

	// Error cases (label mismatch)
	{
		const char* input = "[..";

		Command *output;
		unsigned int output_length = 0;
		int err = tokenize(input, &output, &output_length);

		assert_true(err != 0);
		free(output);
	}
	{
		const char* input = "+,]";

		Command *output;
		unsigned int output_length = 0;
		int err = tokenize(input, &output, &output_length);

		assert_true(err != 0);
		free(output);
	}
	{
		const char* input = "[.]][";

		Command *output;
		unsigned int output_length = 0;
		int err = tokenize(input, &output, &output_length);

		assert_true(err != 0);
		free(output);
	}
}

void test_optimize(void **state)
{
	// No optimizations (level 0)
	{
		const Command input[] = {{T_LABEL, 0}, {T_JUMP, 0}, {T_INC, 1}, {T_LABEL, 1}, {T_JUMP, 1}};
		const unsigned int input_len = 5;

		ProgramSource output;
		int err = optimize(input, input_len, &output, 0);

		assert_true(err == 0);
		assert_true(output.tokens != NULL);
		assert_true(output.length == input_len);
		assert_true(output.no_print_commands == false);
		assert_true(output.no_input_commands == false);

		// Input should be unchanged
		for (unsigned int i = 0; i < input_len; i++) {
			assert_true(input[i].token == output.tokens[i].token);
			assert_true(input[i].value == output.tokens[i].value);
		}
		free(output.tokens);
	}

	// Removing inactive loops (level 1)
	{
		const Command input[] = {{T_LABEL, 0}, {T_JUMP, 0}, {T_INC, 1}, {T_LABEL, 1}, {T_JUMP, 1}};
		const unsigned int input_len = 5;

		ProgramSource output;
		int err = optimize(input, input_len, &output, 1);

		assert_true(err == 0);
		assert_true(output.tokens != NULL);
		assert_true(output.length == 3);
		assert_true(output.no_print_commands == true);
		assert_true(output.no_input_commands == true);

		// First loop should be optimized away
		assert_true(output.tokens[0].token == T_INC);
		assert_true(output.tokens[1].token == T_LABEL);
		assert_true(output.tokens[2].token == T_JUMP);
		assert_true(output.tokens[0].value == 1);
		assert_true(output.tokens[1].value == output.tokens[2].value);

		free(output.tokens);
	}

	// No input commands (level 1)
	{
		const Command input[] = {{T_PRINT, 0}, {T_INC, 1}};
		const unsigned int input_len = 2;

		ProgramSource output;
		int err = optimize(input, input_len, &output, 1);

		assert_true(err == 0);
		assert_true(output.tokens != NULL);
		assert_true(output.length == input_len);
		assert_true(output.no_print_commands == false);
		assert_true(output.no_input_commands == true);

		// Input should be unchanged
		for (unsigned int i = 0; i < input_len; i++) {
			assert_true(input[i].token == output.tokens[i].token);
			assert_true(input[i].value == output.tokens[i].value);
		}
		free(output.tokens);
	}

	// No print commands (level 1)
	{
		const Command input[] = {{T_INC, -1}, {T_INPUT, 0}};
		const unsigned int input_len = 2;

		ProgramSource output;
		int err = optimize(input, input_len, &output, 1);

		assert_true(err == 0);
		assert_true(output.tokens != NULL);
		assert_true(output.length == input_len);
		assert_true(output.no_print_commands == true);
		assert_true(output.no_input_commands == false);

		// Input should be unchanged
		for (unsigned int i = 0; i < input_len; i++) {
			assert_true(input[i].token == output.tokens[i].token);
			assert_true(input[i].value == output.tokens[i].value);
		}
		free(output.tokens);
	}
}
