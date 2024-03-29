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
".section .data\n"
"array:\n"
"        .zero       %u\n"
"buffer:\n"
"        .byte       0\n";

static const char init_section_text[] =
".section .text\n"
".globl _start\n"
"\n";

static const char getchar_body[] =
".type getchar,@function\n"
"getchar:\n"
"        pushq       %%rax\n"
"        xorq        %%rax,%%rax\n"
"        xorq        %%rdi,%%rdi\n"
"        movq        $buffer,%%rsi\n"
"        movq        $1,%%rdx\n"
"        syscall\n"
"        popq        %%rax\n"
"        movb        (buffer),%%cl\n"
"        movb        %%cl,(%%rax)\n"
"        ret\n"
"\n";

static const char putchar_body[] =
".type putchar,@function\n"
"putchar:\n"
"        pushq       %%rax\n"
"        movb        (%%rax),%%bl\n"
"        movb        %%bl,(buffer)\n"
"        movq        $1,%%rax\n"
"        movq        $1,%%rdi\n"
"        movq        $buffer,%%rsi\n"
"        movq        $1,%%rdx\n"
"        syscall\n"
"        popq        %%rax\n"
"        ret\n"
"\n";

static const char start_init[] =
".type _start,@function\n"
"_start:\n"
"        movq        $array,%%rax\n";

static const char start_fini[] =
"\n"
"        movq        $60,%%rax\n"
"        xorq        %%rdi,%%rdi\n"
"        syscall\n";

static const char increment_current_value[] =
"        movb        $%i,%%bl\n"
"        addb        %%bl,(%%rax)\n";
static const char decrement_current_value[] =
"        movb        $%i,%%bl\n"
"        subb        %%bl,(%%rax)\n";
static const char increment_current_pointer[] =
"        addq        $%i,%%rax\n";
static const char decrement_current_pointer[] =
"        subq        $%i,%%rax\n";

static const char label_begin[] =
"\n"
".LB%i:\n"
"        cmpb        $0,(%%rax)\n"
"        je          .LE%i\n";
static const char label_end[] =
"\n"
".LE%i:\n"
"        cmpb        $0,(%%rax)\n"
"        jne         .LB%i\n";

static const char call_getchar[] =
"        call        getchar\n";
static const char call_putchar[] =
"        call        putchar\n";

int
compile_to_obj (char *asm_filename, char *obj_filename)
{
  char *as[] = { "as", "-O2", "--64", "-o", obj_filename, asm_filename, (char *) NULL };

  int err = exec (as);
  if (err != 0)
    error (0, 0, _("error: as returned %i exit status"), err);

  return err;
}

int
link_to_elf (char *obj_filename, char *elf_filename, bool with_debug_info)
{
  char *ld[] = { "ld", "-melf_x86_64", "-O2", "-o", elf_filename, obj_filename, (char *) 0, (char *) 0, (char *) 0 };

  if (with_debug_info)
    ld[countof (ld) - 2] = "--no-gc-sections";
  else
    {
      ld[countof (ld) - 2] = "--gc-sections";
      ld[countof (ld) - 1] = "--strip-all";
    }

  int err = exec (ld);
  if (err != 0)
    error (0, 0, _("error: ld returned %i exit status"), err);

  return err;
}
