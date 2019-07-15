
#ifndef _COMPILER_H
#define _COMPILER_H 1

#include "tokenizer.h"

/* Maximum size of the data array by BrainFuck std.  */
#define DATA_ARRAY_SIZE 30000

extern int str_append (char **str, const char *format, ...)
  __attribute__ ((__format__ (__printf__, 2, 3)));

/* Compiles tokenized source to executable.  */
extern int translate_to_asm (const char *filename,
                             ProgramSource *const source);

#endif /* _COMPILER_H */
