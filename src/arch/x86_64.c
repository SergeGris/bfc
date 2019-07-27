/*  x86_64.c
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

#include "arch.h"

#include <assert.h>
#include <error.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "system.h"

#include "compiler.h"

/* Linux kernel system calls on x86_64 system.  */
static const int syscall_stdin     = 0;
static const int syscall_stdout    = 1;
static const int syscall_sys_exit  = 60;
static const int syscall_sys_read  = 0;
static const int syscall_sys_write = 1;

int
tokens_to_asm (ProgramSource *const source,
               char **final_output,
               size_t *final_output_length)
{
  *final_output = NULL;
  *final_output_length = 0;
  char *output = NULL;

  /* Initialize variables.  */
  str_append (&output, "\
.section    .data\n\
array:\n\
    .zero   %d\n\
buffer:\n\
    .byte   0\n\
\n\
", DATA_ARRAY_SIZE);

  /* Beginning of the code block.  */
  str_append (&output, "\
.section    .text\n\
.global     _start\n\n\
");

  /* Subroutines for I/O.  */
  if (source->have_input_commands)
    {
      str_append (&output, "\
getc:\n\
    push     %%rax\n\
    push     %%rdi\n\
    push     %%rsi\n\
    push     %%rdx\n\
    mov      $%d,%%rax\n\
    mov      $%d,%%rdi\n\
    mov      $buffer,%%rsi\n\
    mov      $1,%%rdx\n\
    syscall\n\
    pop      %%rdx\n\
    pop      %%rsi\n\
    pop      %%rdi\n\
    pop      %%rax\n\
    mov      (buffer),%%cl\n\
    mov      %%cl,(%%rax)\n\
    ret\n\
\n\
", syscall_sys_read, syscall_stdin);
    }
  if (source->have_print_commands)
    {
      str_append (&output, "\
putc:\n\
    push    %%rax\n\
    push    %%rdi\n\
    push    %%rsi\n\
    push    %%rdx\n\
    mov     (%%rax),%%bl\n\
    mov     %%bl,(buffer)\n\
    mov     $%d,%%rax\n\
    mov     $%d,%%rdi\n\
    mov     $buffer,%%rsi\n\
    mov     $1,%%rdx\n\
    syscall\n\
    pop     %%rdx\n\
    pop     %%rsi\n\
    pop     %%rdi\n\
    pop     %%rax\n\
    ret\n\
\n\
", syscall_sys_write, syscall_stdout);
    }

  /* Execution starts at this point.  */
  str_append (&output, "\
_start:\n\
    mov     $array,%%rax\n\
");

  /* Convert tokens to machine code.  */
  int errorcode = 0;
  for (size_t i = 0; i < source->length && errorcode == 0; ++i)
    {
      const Command current = source->tokens[i];
      switch (current.token)
        {
        case T_INC:
          if (current.value > 0)
            {
              str_append (&output, "\
    mov     $%d,%%bl\n\
    add     %%bl,(%%rax)\n\
", +current.value & 0xFF);
            }
          else if (current.value < 0)
            {
              str_append (&output, "\
    mov     $%d,%%bl\n\
    sub     %%bl,(%%rax)\n\
", -current.value & 0xFF);
            }
          else
            {
              /* Command has no effect.  */
              ;
            }
          break;

        case T_POINTER_INC:
          if (current.value > 0)
            {
              str_append (&output, "\
    mov     $%d,%%rbx\n\
    add     %%rbx,%%rax\n\
", +current.value);
            }
          else if (current.value < 0)
            {
              str_append (&output, "\
    mov     $%d,%%rbx\n\
    sub     %%rbx,%%rax\n\
", -current.value);
            }
          else
            {
              /* Command has no effect.  */
              ;
            }
          break;

        case T_LABEL:
          str_append (&output, "\
\n\
label_%d_begin:\n\
    cmpb    $0,(%%rax)\n\
    je      label_%d_end\n\
", current.value, current.value);
          break;
        case T_JUMP:
          str_append (&output, "\
\n\
label_%d_end:\n\
    cmpb    $0,(%%rax)\n\
    jne     label_%d_begin\n\
", current.value, current.value);
          break;

        case T_INPUT:
          if (!source->have_input_commands)
            {
              /* Error: Unexpected token.  */
              errorcode = 202;
            }
          else
            {
              str_append (&output, "\
    call    getc\n\
");
            }
          break;

        case T_OUTPUT:
          if (!source->have_print_commands)
            {
              /* Error: Unexpected token.  */
              errorcode = 202;
            }
          else
            {
              str_append (&output, "\
    call    putc\n\
");
            }
          break;

        case T_COMMENT:
        default:
          break;
        }
    }

  /* Write quit commands.  */
  if (errorcode == 0)
    {
      str_append (&output, "\
\n\
    mov     $%d,%%rax\n\
    xor     %%rdi,%%rdi\n\
    syscall\n\
", syscall_sys_exit);
    }
  else
    {
      free (output);
      return errorcode;
    }

  *final_output = output;
  *final_output_length = strlen (output);

  return 0;
}

int
compile_to_obj (char *asm_filename, char *obj_filename)
{
  char *as[] = { "as", "--64", "-o", obj_filename, asm_filename, (char *) NULL };

  int err = exec (as);
  if (err != 0)
    error (0, 0, _("error: as returned %d exit status"), err);

  return err;
}

int
link_to_elf (char *obj_filename, char *elf_filename)
{
  char *ld[] = { "ld", "-melf_x86_64", "-s", "-o", elf_filename, obj_filename, (char *) NULL };

  int err = exec (ld);
  if (err != 0)
    error (0, 0, _("error: ld returned %d exit status"), err);

  return err;
}
