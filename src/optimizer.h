/*  optimizer.h
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

#ifndef _OPTIMIZER_H
#define _OPTIMIZER_H 1

#include "tokenizer.h"

#include "system.h"

extern int optimize (const Command *const tokens,
                     const size_t tokens_len,
                     ProgramSource *out_result,
                     const unsigned int level)
  __attribute__ ((__nonnull__ (1, 3)));

extern int tokenize_and_optimize (const char *const source,
                                  ProgramSource *out_result,
                                  const unsigned int level)
  __attribute__ ((__nonnull__ (1, 2)));
  
#endif /* _OPTIMIZER_H */