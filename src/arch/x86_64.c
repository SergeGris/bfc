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

static const char init_variables[] =
".section    .data\n"
"array:\n"
"    .zero   %u\n"
"buffer:\n"
"    .byte   0\n";

static const char init_section_text[] =
".section    .text\n"
".globl      _start\n"
"\n";

static const char getchar_body[] =
".type getchar,@function\n"
"getchar:\n"
"    pushq   %%rax\n"
"    pushq   %%rdi\n"
"    pushq   %%rsi\n"
"    pushq   %%rdx\n"
"    xorq    %%rax,%%rax\n"
"    xorq    %%rdi,%%rdi\n"
"    movq    $buffer,%%rsi\n"
"    movq    $1,%%rdx\n"
"    syscall\n"
"    popq    %%rdx\n"
"    popq    %%rsi\n"
"    popq    %%rdi\n"
"    popq    %%rax\n"
"    movb    (buffer),%%cl\n"
"    movb    %%cl,(%%rax)\n"
"    retq\n"
"\n";

static const char putchar_body[] =
".type putchar,@function\n"
"putchar:\n"
"    pushq   %%rax\n"
"    pushq   %%rdi\n"
"    pushq   %%rsi\n"
"    pushq   %%rdx\n"
"    movb    (%%rax),%%bl\n"
"    movb    %%bl,(buffer)\n"
"    movq    $1,%%rax\n"
"    movq    $1,%%rdi\n"
"    movq    $buffer,%%rsi\n"
"    movq    $1,%%rdx\n"
"    syscall\n"
"    popq    %%rdx\n"
"    popq    %%rsi\n"
"    popq    %%rdi\n"
"    popq    %%rax\n"
"    retq\n"
"\n";

static const char start_begin_and_init_array[] =
".type _start,@function\n"
"_start:\n"
"    movq    $array,%%rax\n";

static const char start_end[] =
"\n"
"    movq    $60,%%rax\n"
"    xorq    %%rdi,%%rdi\n"
"    syscall\n";

static const char increment_current_value[] =
"    movb    $%d,%%bl\n"
"    addb    %%bl,(%%rax)\n";
static const char decrement_current_value[] =
"    movb    $%d,%%bl\n"
"    subb    %%bl,(%%rax)\n";
static const char increment_current_pointer[] =
"    addq    $%d,%%rax\n";
static const char decrement_current_pointer[] =
"    subq    $%d,%%rax\n";

static const char label_begin[] =
"\n"
".LB%d:\n"
"    cmpb    $0,(%%rax)\n"
"    je      .LE%d\n";
static const char label_end[] =
"\n"
".LE%d:\n"
"    cmpb    $0,(%%rax)\n"
"    jne     .LB%d\n";

static const char call_getchar[] =
"    callq   getchar\n";
static const char call_putchar[] =
"    callq   putchar\n";

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
