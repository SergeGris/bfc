/*  arch.c
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

#if defined(__amd64__) || defined(__x86_64__) || defined(_AMD64_)
# include "arch/x86_64.c"
#elif defined(__i386__)
# include "arch/x86.c"
#elif defined(__arm__)
# include "arch/arm.c"
#elif defined(__aarch64__)
# include "arch/arm64.c"
#elif defined(__m68k__)
# include "arch/m68k.c"
#endif

#include <unistd.h>

#include "die.h"

int
exec (char **arg)
{
  int status = -1;
  pid_t pid  = vfork ();

  if (pid < 0)
    /* Fork error.  */
    die (EXIT_FAILURE, errno, "%s", arg[0]);
  else if (pid == 0)
    /* Child process.  */
    execvp (arg[0], arg);
  else /* pid > 0 */
    /* Parent process.  */
    wait (&status);

  return status;
}
