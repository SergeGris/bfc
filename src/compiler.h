/*  compiler.h
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

#ifndef _COMPILER_H
#define _COMPILER_H 1

#include "tokenizer.h"

#include "system.h"

/* Maximum size of the data array by BrainFuck std.  */
#define DATA_ARRAY_SIZE 30000

extern int str_append (char **str, size_t *length, const char *format, ...)
  __attribute__ ((__format__ (__printf__, 3, 4), __nonnull__ (1, 2, 3)));

/* Compiles tokenized source to executable.  */
extern int translate_to_asm (const char *filename,
                             ProgramSource *const source)
  __attribute__ ((__nonnull__ (1, 2)));

#endif /* _COMPILER_H */
