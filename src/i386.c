/*  i386.c
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
"        pushl       %%eax\n"
"        movl        $3,%%eax\n"
"        movl        $0,%%ebx\n"
"        movl        $buffer,%%ecx\n"
"        movl        $1,%%edx\n"
"        int         $0x80\n"
"        popl        %%eax\n"
"        movb        (buffer),%%cl\n"
"        movb        %%cl,(%%eax)\n"
"        ret\n"
"\n";

static const char putchar_body[] =
".type getchar,@function\n"
"putchar:\n"
"        pushl       %%eax\n"
"        movb        (%%eax),%%bl\n"
"        movb        %%bl,(buffer)\n"
"        movl        $4,%%eax\n"
"        movl        $1,%%ebx\n"
"        movl        $buffer,%%ecx\n"
"        movl        $1,%%edx\n"
"        int         $0x80\n"
"        popl        %%eax\n"
"        ret\n"
"\n";

static const char start_init[] =
".type _start,@function\n"
"_start:\n"
"        movl        $array,%%eax\n";

static const char start_fini[] =
"\n"
"        movl        $1,%%eax\n"
"        xorl        %%ebx,%%ebx\n"
"        int         $0x80\n";
static const char increment_current_value[] =
"        movb        $%i,%%bl\n"
"        addb        %%bl,(%%eax)\n";
static const char decrement_current_value[] =
"        movb        $%i,%%bl\n"
"        subb        %%bl,(%%eax)\n";
static const char increment_current_pointer[] =
"        addl        $%i,%%eax\n";
static const char decrement_current_pointer[] =
"        subl        $%i,%%eax\n";

static const char label_begin[] =
"\n"
".LB%i:\n"
"        cmpb        $0,(%%eax)\n"
"        je          .LE%i\n";
static const char label_end[] =
"\n"
".LE%i:\n"
"        cmpb        $0,(%%eax)\n"
"        jne         .LB%i\n";

static const char call_getchar[] =
"        call        getchar\n";
static const char call_putchar[] =
"        call        putchar\n";

int
compile_to_obj (char *asm_filename, char *obj_filename)
{
  char *as[] = { "as", "-O2", "--32", "-o", obj_filename, asm_filename, (char *) NULL };

  int err = exec (as);
  if (err != 0)
    error (0, 0, _("error: as returned %i exit status"), err);

  return err;
}

int
link_to_elf (char *obj_filename, char *elf_filename, bool with_debug_info)
{
  char *ld[] = { "ld", "-melf_i386", "-O2", "-o", elf_filename, obj_filename, (char *) 0, (char *) 0, (char *) 0 };

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
