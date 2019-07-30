/*  x86.c
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

/* Linux kernel system calls on x86 system.  */
static const int syscall_stdin     = 0;
static const int syscall_stdout    = 1;
static const int syscall_sys_exit  = 1;
static const int syscall_sys_read  = 3;
static const int syscall_sys_write = 4;

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
.globl      _start\n\n\
");

  /* Subroutines for I/O.  */
  if (source->have_getchar_commands)
    {
      str_append (&output, "\
.type getchar,@function\n\
getchar:\n\
    pushl   %%eax\n\
    pushl   %%ebx\n\
    pushl   %%ecx\n\
    pushl   %%edx\n\
    movl    $%d,%%eax\n\
    movl    $%d,%%ebx\n\
    movl    $buffer,%%ecx\n\
    movl    $1,%%edx\n\
    int     $0x80\n\
    popl    %%edx\n\
    popl    %%ecx\n\
    popl    %%ebx\n\
    popl    %%eax\n\
    movb    (buffer),%%cl\n\
    movb    %%cl,(%%eax)\n\
    retl\n\
\n\
", syscall_sys_read, syscall_stdin);
    }
  if (source->have_putchar_commands)
    {
      str_append (&output, "\
.type getchar,@function\n\
putchar:\n\
    pushl   %%eax\n\
    pushl   %%ebx\n\
    pushl   %%ecx\n\
    pushl   %%edx\n\
    xorl    %%ebx,%%ebx\n\
    movb    (%%eax),%%bl\n\
    movb    %%bl,(buffer)\n\
    movl    $%d,%%eax\n\
    movl    $%d,%%ebx\n\
    movl    $buffer,%%ecx\n\
    movl    $1,%%edx\n\
    int     $0x80\n\
    popl    %%edx\n\
    popl    %%ecx\n\
    popl    %%ebx\n\
    popl    %%eax\n\
    retl\n\
\n\
", syscall_sys_write, syscall_stdout);
    }

  /* Execution starts at this point.  */
  str_append (&output, "\
_start:\n\
    movl    $array,%%eax\n\
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
    addb    %%bl,(%%eax)\n\
", +current.value & 0xFF);
            }
          else if (current.value < 0)
            {
              str_append (&output, "\
    movb    $%d,%%bl\n\
    subb    %%bl,(%%eax)\n\
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
    movl    $%d,%%ebx\n\
    addl    %%ebx,%%eax\n\
", +current.value);
            }
          else if (current.value < 0)
            {
              str_append (&output, "\
    movl    $%d,%%ebx\n\
    subl    %%ebx,%%eax\n\
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
    cmpb    $0,(%%eax)\n\
    je      .LE%d\n\
", current.value, current.value);
          break;
        case T_JUMP:
          str_append (&output, "\
\n\
.LE%d:\n\
    cmpb    $0,(%%eax)\n\
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
    calll   getchar\n\
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
    calll   putchar\n\
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
    movl    $%d,%%eax\n\
    xorl    %%ebx,%%ebx\n\
    int     $0x80\n\
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
  char *as[] = { "as", "--32", "-o", obj_filename, asm_filename, (char *) NULL };

  int err = exec (as);
  if (err != 0)
    error (0, 0, _("error: as returned %d exit status"), err);

  return err;
}

int
link_to_elf (char *obj_filename, char *elf_filename)
{
  char *ld[] = { "ld", "-melf_i386", "-O2", "--gc-sections", "--strip-all", "-o", elf_filename, obj_filename, (char *) NULL };

  int err = exec (ld);
  if (err != 0)
    error (0, 0, _("error: ld returned %d exit status"), err);

  return err;
}
