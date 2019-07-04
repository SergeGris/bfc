/* system-dependent definitions for coreutils
   Copyright (C) 2019 Sergey Sushilin

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Include this file _after_ system headers if possible.  */

#ifndef _SYSTEM_H
#define _SYSTEM_H 1

#include <sys/cdefs.h>

#if ((defined(_WIN32) || defined(__WIN32__)) && !defined(__CYGWIN__)) && !defined(__windows__)
# define __windows__ 1
#endif

/* By default, colon separates directories in a path.  */
#if !defined(PATH_SEPARATOR)
# if __windows__ || HAVE_DOS_BASED_FILE_SYSTEM
#  define PATH_SEPARATOR ';'
# else
#  define PATH_SEPARATOR ':'
# endif
#endif

/* These should be phased out in favor of IS_DIR_SEPARATOR, where possible.  */
#if !defined(DIR_SEPARATOR)
# if __windows__ || HAVE_DOS_BASED_FILE_SYSTEM
#  define DIR_SEPARATOR '\\'
# else
#  define DIR_SEPARATOR '/'
# endif
#endif /* DIR_SEPARATOR */

#if !defined(__attribute__)
# if !defined(__GNUC__) || (__GNUC__ < 2 || (__GNUC__ == 2 && __GNUC_MINOR__ < 8))
#  define __attribute__(x) /* empty */
# endif
#endif /* __attribute__ */

#include <stddef.h>

#include <alloca.h>

#if HAVE_STDBOOL_H
# include <stdbool.h>
#endif

/* Declare file status routines and bits.  */
#if HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif

#if HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif

#include <unistd.h>

#include "pathmax.h"
#if !defined(PATH_MAX)
# define PATH_MAX 8192
#endif

#include "configmake.h"

#include <string.h>
#include <errno.h>

/* Some systems don't define this; POSIX mentions it but says it is
   obsolete.  gnulib defines it, but only on native Windows systems,
   and there only because MSVC 10 does.  */
#if !defined(ENODATA)
# define ENODATA (-1)
#endif

#include <stdbool.h>
#include <stdlib.h>
#define EXIT_TROUBLE 2

#include "version.h"

#include "exitfail.h"

/* Set exit_failure to STATUS if that's not the default already.  */
static inline void
initialize_exit_failure (int status)
{
  if (status != EXIT_FAILURE && status != exit_failure)
    exit_failure = status;
}


#include <fcntl.h>

#include <inttypes.h>


/* Redirection and wildcarding when done by the utility itself.
   Generally a noop, but used in particular for OS/2.  */
#if !defined(initialize_main)
# if !defined(__OS2__)
#  define initialize_main(ac, av) /* empty */
# else
#  define initialize_main(ac, av) \
  do { _wildcard (ac, av); _response (ac, av); } while (0)
# endif
#endif

#include "stat-macros.h"

#include <ctype.h>

#include "gettext.h"

#if ENABLE_NLS
/* On some systems, things go awry when <libintl.h> comes after <locale.h>.  */
# include <libintl.h>
# include <locale.h>
# define _(String) gettext (String)
# if defined(gettext_noop)
#  define N_(String) gettext_noop (String)
# else
#  define N_(String) (String)
# endif
# define S_(Msgid1, Msgid2, n) ngettext (Msgid1, Msgid2, n)
#else /* !ENABLE_NLS */
/* Include <locale.h> first to avoid conflicts with these macros.  */
# include <locale.h>
# undef gettext
# undef ngettext
# undef textdomain
# undef bindtextdomain

# define gettext(Msgid) (Msgid)
# define ngettext(Msgid1, Msgid2, n) (n == 1 ? Msgid1 : Msgid2)
# define textdomain(Domainname) do { } while (0)
# define bindtextdomain(Domainname, Dirname) do { } while (0)

# define _(String) (String)
# define N_(String) (String)
# define S_(Msgid1, Msgid2, n) (n == 1 ? Msgid1 : Msgid2)
#endif /* ENABLE_NLS */

/* Return a value that pluralizes the same way that N does, in all
   languages we know of.  */
static inline unsigned long int
select_plural (uintmax_t n)
{
  /* Reduce by a power of ten, but keep it away from zero.
     The gettext manual says 1000000 should be safe.  */
  enum { PLURAL_REDUCER = 1000000 };
  return (n <= ULONG_MAX ? n : n % PLURAL_REDUCER + PLURAL_REDUCER);
}

#define STREQ(a, b)        (strcmp ((a), (b)) == 0)
#define STREQ_LEN(a, b, n) (strncmp ((a), (b), (n)) == 0)
#define STRPREFIX(a, b)    (strncmp ((a), (b), strlen (b)) == 0)
#define STRSUFFIX(a, b)    ((strlen (a) < strlen (b)) ? false : strcmp ((a) + strlen (a) - strlen (b), (b)) == 0)

#include "verify.h"

#define HELP_OPTION_DESCRIPTION \
  _("      --help           display this help and exit\n")
#define VERSION_OPTION_DESCRIPTION \
  _("      --version        output version information and exit\n")

/* Check for errors on write.  */
#include "closeout.h"

#define emit_bug_reporting_address unused__emit_bug_reporting_address
#include "version-etc.h"
#undef emit_bug_reporting_address

#include "propername.h"
/* Define away proper_name (leaving proper_name_utf8, which affects far
   fewer programs), since it is not worth the cost of adding ~17KB to
   the x86_64 text size of every single program.  This avoids a 40%
   (almost ~2MB) increase in the on-disk space utilization for the set
   of the 100 binaries.  */
#define proper_name(x) (x)

#include "progname.h"

/* Use this to suppress gcc's '...may be used before initialized' warnings.  */
#if defined(lint)
# define IF_LINT(Code) Code
#else
# define IF_LINT(Code) /* empty */
#endif /* lint */

#if !defined(FALLTHROUGH)
# if defined(__GNUC__) && __GNUC__ >= 7
#  define FALLTHROUGH __attribute__ ((__fallthrough__))
# else
#  define FALLTHROUGH ((void) 0)
# endif
#endif /* FALLTHROUGH */


#include <stdnoreturn.h>


/* __builtin_expect(CONDITION, EXPECTED_VALUE) evaluates to CONDITION, but notifies the compiler that
   the most likely value of CONDITION is EXPECTED_VALUE.  This feature was added at some point
   between 2.95 and 3.0.  Let's use 3.0 as the lower bound for now.  */
#if (__GNUC__ < 3) && !defined(__builtin_expect)
# define __builtin_expect(condition, expected_value) (condition)
#endif

#define   likely(condition) __builtin_expect (!!(condition), true)
#define unlikely(condition) __builtin_expect (!!(condition), false)

/* Use a macro rather than an inline function, as this references
   the global program_name, which causes dynamic linking issues
   in libstdbuf.so on some systems where unused functions
   are not removed by the linker.  */
#define emit_try_help() \
  do \
    { \
      fprintf (stderr, _("Try '%s --help' for more information.\n"), \
               program_name); \
    } \
  while (0)


static inline char *
bad_cast (const char *s)
{
  return (char *) s;
}

/* How coreutils quotes filenames, to minimize use of outer quotes,
   but also provide better support for copy and paste when used.  */
#include "quotearg.h"

/* Use these to shell quote only when necessary,
   when the quoted item is already delimited with colons.  */
#define quotef(arg) \
  quotearg_n_style_colon (0, shell_escape_quoting_style, arg)
#define quotef_n(n, arg) \
  quotearg_n_style_colon (n, shell_escape_quoting_style, arg)

/* Use these when there are spaces around the file name,
   in the error message.  */
#define quoteaf(arg) \
  quotearg_style (shell_escape_always_quoting_style, arg)
#define quoteaf_n(n, arg) \
  quotearg_n_style (n, shell_escape_always_quoting_style, arg)

#endif /* _SYSTEM_H */
