/*  arch.h
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

#ifndef _ARCH_H
#define _ARCH_H 1

#include <stddef.h>

#include "tokenizer.h"

/* Compiles tokenized source to assembly source code.  */
extern void tokens_to_asm (ProgramSource *const source,
                           char **final_output,
                           size_t *final_output_length);
extern int exec (char **arg);
extern int compile_to_obj (char *asm_fn, char *obj_fn);
extern int link_to_elf (char *obj_fn, char *elf_fn, bool with_debug_info);

#endif /* _ARCH_H */
