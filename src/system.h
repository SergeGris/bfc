/*  system-dependent definitions for BrainFuck Compiler
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

/* Include this file _after_ system headers if possible.  */

#ifndef _SYSTEM_H
#define _SYSTEM_H 1

#include <alloca.h>

#include <sys/cdefs.h>

#if !defined(__attribute__)
# if !defined(__GNUC__) || (__GNUC__ < 2 || (__GNUC__ == 2 && __GNUC_MINOR__ < 8))
#  define __attribute__(x) /* empty */
# endif
#endif /* __attribute__ */

#include <stdint.h>
typedef  int8_t  i8;
typedef  int16_t i16;
typedef  int32_t i32;
typedef  int64_t i64;
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#include <errno.h>

/* Some systems do not define this; POSIX mentions it but says it is
   obsolete.  gnulib defines it, but only on native Windows systems,
   and there only because MSVC 10 does.  */
#if !defined(ENODATA)
# define ENODATA (-1)
#endif

#include <stdbool.h>
#include <stdlib.h>
#define EXIT_TROUBLE 2

#include "version.h"

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

#include <limits.h>
#include <inttypes.h>

/* Return a value that pluralizes the same way that N does, in all
   languages we know of.  */
static inline __attribute__ ((__const__)) unsigned long int
select_plural (uintmax_t n)
{
  /* Reduce by a power of ten, but keep it away from zero.
     The gettext manual says 1000000 should be safe.  */
  enum { PLURAL_REDUCER = 1000000 };
  return (n <= ULONG_MAX ? n : n % PLURAL_REDUCER + PLURAL_REDUCER);
}

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

#define __same_type(a, b) __builtin_types_compatible_p (typeof (a), typeof (b))

/* Get the number of elements in array.  */
#define countof(arr) (sizeof (arr) / sizeof ((arr)[0]))

/* __builtin_expect(CONDITION, EXPECTED_VALUE) evaluates to CONDITION, but notifies the compiler that
   the most likely value of CONDITION is EXPECTED_VALUE.  */
#if (!defined(__GNUC__) || (__GNUC__ <= 2 && __GNUC_MINOR__ < 96)) && !defined(__builtin_expect)
# define __builtin_expect(condition, expected_value) (condition)
#endif

#define   likely(condition) __builtin_expect ((condition) != 0, true)
#define unlikely(condition) __builtin_expect ((condition) != 0, false)

#define emit_try_help() \
  do \
    { \
      fprintf (stderr, _("Try '%s --help' for more information.\n"), \
               program_name); \
    } \
  while (0)


static inline __attribute__ ((__const__)) char *
bad_cast (const char *s)
{
  return (char *) s;
}

void usage (int status) __attribute__ ((__noreturn__));

/* How coreutils quotes filenames, to minimize use of outer quotes,
   but also provide better support for copy and paste when used.  */
#include "quote.h"
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
