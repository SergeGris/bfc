
#include "x86-asm.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "compiler.h"

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
.global     _start\n\n\
");

  /* Subroutines for I/O */
  if (source->have_print_commands)
    {
      str_append (&output, "\
print_char:\n\
    push    %%eax\n\
    push    %%ebx\n\
    push    %%ecx\n\
    push    %%edx\n\
    xor     %%ebx,%%ebx\n\
    mov     (%%eax),%%bl\n\
    mov     %%bl,(buffer)\n\
    mov     $%d,%%eax\n\
    mov     $%d,%%ebx\n\
    mov     $buffer,%%ecx\n\
    mov     $1,%%edx\n\
    int     $0x80\n\
    pop     %%edx\n\
    pop     %%ecx\n\
    pop     %%ebx\n\
    pop     %%eax\n\
    ret\n\
\n\
", syscall_sys_write, syscall_stdout);
    }
  if (source->have_input_commands)
    {
      str_append (&output, "\
input_char:\n\
    push    %%eax\n\
    push    %%ebx\n\
    push    %%ecx\n\
    push    %%edx\n\
    mov     $%d,%%eax\n\
    mov     $%d,%%ebx\n\
    mov     $buffer,%%ecx\n\
    mov     $1,%%edx\n\
    int     $0x80\n\
    pop     %%edx\n\
    pop     %%ecx\n\
    pop     %%ebx\n\
    pop     %%eax\n\
    mov     (buffer),%%cl\n\
    mov     %%cl,(%%eax)\n\
    ret\n\
\n\
", syscall_sys_read, syscall_stdin);
    }

  /* Execution starts at this point.  */
  str_append (&output, "\
_start:\n\
    mov     $array,%%eax\n\
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
    add     %%bl,(%%eax)\n\
", current.value & 0xFF);
            }
          else if (current.value < 0)
            {
              str_append (&output, "\
    mov     $%d,%%bl\n\
    sub     %%bl,(%%eax)\n\
", (-current.value) & 0xFF);
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
    mov     $%d,%%ebx\n\
    add     %%ebx,%%eax\n\
", current.value);
            }
          else if (current.value < 0)
            {
              str_append (&output, "\
    mov     $%d,%%ebx\n\
    sub     %%ebx,%%eax\n\
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
    cmpb    $0,(%%eax)\n\
    je      label_%d_end\n\
", current.value, current.value);
          break;
        case T_JUMP:
          str_append (&output, "\
\n\
label_%d_end:\n\
    cmpb    $0,(%%eax)\n\
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
    call    input_char\n\
");
            }
          break;

        case T_PRINT:
          if (!source->have_print_commands)
            {
              /* Error: Unexpected token.  */
              errorcode = 202;
            }
          else
            {
              str_append (&output, "\
    call    print_char\n\
");
            }
          break;

        default:
          break;
        }
    }

  /* Write quit commands.  */
  if (errorcode == 0)
    {
      str_append (&output, "\
\n\
    mov     $%d,%%eax\n\
    mov     $0,%%ebx\n\
    int     $0x80\n\
", syscall_sys_exit);

#if 0 /* for FreeBSD */
      str_append (&output, "\
\n\
    push    $0\n\
    mov     $%d,%%eax\n\
    push    %%eax\n\
    int     $0x80\n\
", syscall_sys_exit);
#endif
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
