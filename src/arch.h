
#ifndef _ARCH_H
#define _ARCH_H 1

#include <stddef.h>

#include "tokenizer.h"

/* Compiles tokenized source to assembly source code.  */
extern int tokens_to_asm (ProgramSource *const source,
                          char **final_output,
                          size_t *final_output_length);

extern int compile_to_obj (char *, char *);
extern int link_to_elf (char *, char *);

#endif /* _ARCH_H */
