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
buffer:\n"

#if STRICT_MULLER
"\
    .byte   0\n"
#else
"\
    .long   0\n"
#endif

, DATA_ARRAY_SIZE);

  /* Beginning of the code block.  */
  str_append (&output, "\
.section    .text\n\
.globl      _start\n\n\
");

  /* Subroutines for I/O.  */
  if (source->have_getchar_commands)
    {
      str_append (&output, "\
.type getchar,@function\n\
getchar:\n\
    pushq   %%rax\n\
    pushq   %%rdi\n\
    pushq   %%rsi\n\
    pushq   %%rdx\n\
    movq    $%d,%%rax\n\
    movq    $%d,%%rdi\n\
    movq    $buffer,%%rsi\n\
    movq    $1,%%rdx\n\
    syscall\n\
    popq    %%rdx\n\
    popq    %%rsi\n\
    popq    %%rdi\n\
    popq    %%rax\n\
    movb    (buffer),%%cl\n\
    movb    %%cl,(%%rax)\n\
    retq\n\
\n\
", syscall_sys_read, syscall_stdin);
    }
  if (source->have_putchar_commands)
    {
      str_append (&output, "\
.type putchar,@function\n\
putchar:\n\
    pushq   %%rax\n\
    pushq   %%rdi\n\
    pushq   %%rsi\n\
    pushq   %%rdx\n\
    movb    (%%rax),%%bl\n\
    movb    %%bl,(buffer)\n\
    movq    $%d,%%rax\n\
    movq    $%d,%%rdi\n\
    movq    $buffer,%%rsi\n\
    movq    $1,%%rdx\n\
    syscall\n\
    popq    %%rdx\n\
    popq    %%rsi\n\
    popq    %%rdi\n\
    popq    %%rax\n\
    retq\n\
\n\
", syscall_sys_write, syscall_stdout);
    }

  /* Execution starts at this point.  */
  str_append (&output, "\
.type _start,@function\n\
_start:\n\
    movq    $array,%%rax\n\
");

  /* Convert tokens to machine code.  */
  int errorcode = 0;
  for (size_t i = 0; i < source->length && errorcode == 0; ++i)
    {
      const Command current = source->tokens[i];
      switch (current.token)
        {
        case T_INCDEC:
          if (current.value > 0)
            {
              str_append (&output, "\
    movb    $%d,%%bl\n\
    addb    %%bl,(%%rax)\n\
", +current.value & 0xFF);
            }
          else if (current.value < 0)
            {
              str_append (&output, "\
    movb    $%d,%%bl\n\
    subb    %%bl,(%%rax)\n\
", -current.value & 0xFF);
            }
          else
            {
              /* Command has no effect.  */
              ;
            }
          break;

        case T_POINTER_INCDEC:
          if (current.value > 0)
            {
              str_append (&output, "\
    movq    $%d,%%rbx\n\
    addq    %%rbx,%%rax\n\
", +current.value);
            }
          else if (current.value < 0)
            {
              str_append (&output, "\
    movq    $%d,%%rbx\n\
    subq    %%rbx,%%rax\n\
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
.LB%d:\n\
    cmpb    $0,(%%rax)\n\
    je      .LE%d\n\
", current.value, current.value);
          break;
        case T_JUMP:
          str_append (&output, "\
\n\
.LE%d:\n\
    cmpb    $0,(%%rax)\n\
    jne     .LB%d\n\
", current.value, current.value);
          break;

        case T_GETCHAR:
          if (!source->have_getchar_commands)
            {
              /* Error: Unexpected token.  */
              errorcode = 202;
            }
          else
            {
              str_append (&output, "\
    callq   getchar\n\
");
            }
          break;

        case T_PUTCHAR:
          if (!source->have_putchar_commands)
            {
              /* Error: Unexpected token.  */
              errorcode = 202;
            }
          else
            {
              str_append (&output, "\
    callq   putchar\n\
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
    movq    $%d,%%rax\n\
    xorq    %%rdi,%%rdi\n\
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
  char *ld[] = { "ld", "-melf_x86_64", "-O2", "--gc-sections", "--strip-all", "-o", elf_filename, obj_filename, (char *) NULL };

  int err = exec (ld);
  if (err != 0)
    error (0, 0, _("error: ld returned %d exit status"), err);

  return err;
}
