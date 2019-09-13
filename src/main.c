/*  main.c
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <locale.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <getopt.h>

#include "system.h"

#include "arch.h"
#include "compiler.h"
#include "tokenizer.h"
#include "optimizer.h"

#include "configmake.h"
#include "die.h"
#include "filenamecat.h"
#include "long-options.h"
#include "tempname.h"
#include "xalloc.h"
#include "xdectoint.h"
#include "xstrndup.h"

/* The official name of this program (e.g., no 'g' prefix).  */
#define PROGRAM_NAME "bfc"

#define AUTHORS \
  proper_name ("Sergey Sushilin")

static char *
read_file (const char *filename)
{
  char *buf = NULL;
  long  len = 0L;
  FILE *fp  = NULL;

  if ((fp = fopen (filename, "r")) == NULL
    || fseek (fp, 0L, SEEK_END) < 0
    || (len = ftell (fp))       < 0
    || fseek (fp, 0L, SEEK_SET) < 0)
    goto lfail;

  buf = xmalloc (len + 1);
  size_t res = fread (buf, sizeof (char), len, fp);
  if (res != len
   || fclose (fp) != 0)
    goto lfail;
  buf[len] = '\0';
  return buf;

lfail:
  error (0, errno, "%s", quotef (filename));
  if (buf != NULL)
    free (buf);
  return NULL;
}

static const char *
change_extension (char *filename, const char *new_extension)
{
  if (unlikely (filename == NULL || *filename == '\0'))
    return "";
  if (unlikely (new_extension == NULL))
    return filename;

  size_t len = strlen (filename);

  for (char *tmp = filename + len - 1; tmp >= filename; tmp--)
    {
      if (*tmp == '.')
        {
          while ((*tmp++ = *new_extension++) != '\0')
            ;
          *tmp = '\0';
          return filename;
        }
    }

  char *tmp = filename + len;
  while ((*tmp++ = *new_extension++) != '\0')
    ;
  *tmp = '\0';

  return filename;
}

static char *
cut_path (char *str)
{
  if (*str == '\0')
    return str;

  char *tmp = strchr (str, '\0');

  while (tmp > str && *(tmp - 1) != '/')
    tmp--;

  return tmp;
}

static bool do_assemble                = true;
static bool do_link                    = true;
static bool save_temps                 = false;
static unsigned int optimization_level = 0;
static unsigned int files_to_compile   = 0;

void
usage (int status)
{
  if (status != EXIT_SUCCESS)
    emit_try_help ();
  else
    {
      printf (_("\
Usage: %s [-sco:O:]\n"), program_name);
      puts (_("\
  --help                   Display this information and exit.\n\
  --version                Display compiler's version and exit.\n\
  --save-temps             Do not delete temporary files.\n\
  -s                       Compile only, do not assemble or link.\n\
  -c                       Compile and assemble, but do not link.\n\
  -o <file>                Place the output into <file>.\n\
  -On                      Level of optimization, default is 0."));
    }

  exit (status);
}

enum
{
  SAVE_TEMPS_OPTION = CHAR_MAX + 1
};

static const struct option long_options[] =
{
  {"save-temps", no_argument, NULL, SAVE_TEMPS_OPTION},
  {NULL, 0, NULL, '\0'}
};

/* Advise the user about invalid usages like "bfc -foo.bf" if the file
   "-foo.bf" exists, assuming ARGC and ARGV are as with 'main'.  */
static void
diagnose_leading_hyphen (int argc, char **argv)
{
  /* OPTIND is unreliable, so iterate through the arguments looking
     for a file name that looks like an option.  */

  struct stat st;
  for (int i = 1; i < argc; i++)
    {
      const char *arg = argv[i];
      if (arg[0] == '-' && arg[1] != '\0' && lstat (arg, &st) == 0)
        {
          fprintf (stderr,
                   _("Try '%s ./%s' to get a compile the file %s.\n"),
                   argv[0],
                   quotearg_n_style (1, shell_escape_quoting_style, arg),
                   quoteaf (arg));
        }
    }
}

void
parseopt (int argc, char **argv)
{
  parse_long_options (argc, argv, PROGRAM_NAME, PACKAGE_NAME, Version, usage, AUTHORS,
                      (const char *) NULL);

  int optc = -1;

  while ((optc = getopt_long (argc, argv, "o:O:cs", long_options, NULL)) != -1)
    switch (optc)
      {
        /*
      case 'o':
        out_filename = optarg;
        break;*/

      case 'O':
        if (*optarg == '\0' || *(optarg + 1) != '\0' || (*optarg < '0' && *optarg > '9'))
          error (0, 0, _("invalid optimization level %s"), optarg);
        optimization_level = *optarg - '0';
        if (optimization_level > 1)
          {
            /* Maximum optimization level is two, if after '-O' costs 2
               or greater number, then replace it by 1.  */
            optimization_level = 1;
          }
        break;
      case 'c':
        do_assemble = true;
        do_link = false;
        break;
      case 's':
        do_assemble = do_link = false;
        break;
      case SAVE_TEMPS_OPTION:
        save_temps = true;
        break;
      default:
        diagnose_leading_hyphen (argc, argv);
        usage (EXIT_FAILURE);
      }

  errno = 0;
  char **operand_lim = argv + argc;
  for (char **operandp = argv + optind; operandp < operand_lim; operandp++)
    {
      char *file = *operandp;
      struct stat st;
      if (stat (file, &st) == 0)
        files_to_compile++;
      else if (errno != 0)
        die (EXIT_TROUBLE, errno, "%s", quoteaf (file));
    }

  if (files_to_compile == 0)
    die (EXIT_FAILURE, 0, _("fatal error: no input files."));
}

static size_t
count_consecutive_X_s (const char *s, size_t len)
{
  size_t n = 0;
  while (len != 0 && s[len - 1] == 'X')
    {
      n++;
      len--;
    }
  return n;
}
static int
mktemp_len (char *tmpl, size_t suff_len, size_t x_len, bool create)
{
  return gen_tempname_len (tmpl, suff_len, 0, create ? GT_FILE : GT_NOCREATE, x_len);
}

static char *
mktmp (const char *template, size_t suff_len)
{
  char *env = getenv ("TMPDIR");
  char *tmpdir = ((env != NULL && *env != '\0') ? env : "/tmp");

  char *tmpfile = file_name_concat (tmpdir, template, NULL);

  char *x_suff = strchr (template, 'X');
  if (x_suff == NULL)
    die (EXIT_TROUBLE, 0, _("invalid template %s, template must end by three or more X's"), quote (template));

  size_t x_len = count_consecutive_X_s (x_suff, strlen (x_suff) - suff_len);
  if (x_len < 3)
    die (EXIT_TROUBLE, 0, _("too few X's in template %s, template must end by three or more X's"), quote (template));

  int fd = mktemp_len (tmpfile, suff_len, x_len, true);

  if (fd < 0 || close (fd) != 0)
    die (EXIT_FAILURE, errno, _("failed to create file via template %s"), quote (template));

  return tmpfile;
}

static int
compile_file (char *filename)
{
  char *out_asm = NULL;
  char *out_obj = NULL;
  char *clean_filename = cut_path (filename);
  const size_t clean_filename_len = strlen (clean_filename);

  if (save_temps || !do_link)
    {
      if (!save_temps)
        {
          if (!do_assemble)
            {
              out_asm = xstrndup (clean_filename, clean_filename_len - 1);
              change_extension (out_asm, ".s");
            }
          else
            {
              out_asm = mktmp ("bfc-XXXXXXXXXXXX.s", 2);

              out_obj = xstrndup (clean_filename, clean_filename_len - 1);
              change_extension (out_obj, ".o");
            }
        }
      else
        {
          out_asm = xstrndup (clean_filename, clean_filename_len - 1);
          out_obj = xstrndup (clean_filename, clean_filename_len - 1);

          change_extension (clean_filename, ".s");
          change_extension (clean_filename, ".o");
        }
    }
  else
    {
      out_asm = mktmp ("bfc-XXXXXXXXXXXX.s", 2);
      out_obj = mktmp ("bfc-XXXXXXXXXXXX.o", 2);
    }

  char *out_filename = xmalloc (clean_filename_len + 1);
  snprintf (out_filename, clean_filename_len + 1, "%s", clean_filename);
  change_extension (out_filename, "");

  ProgramSource tokenized_source;

  /* Open file.  */
  char *source = read_file (filename);
  if (source == NULL)
    die (EXIT_FAILURE, 0, _("fatal error: failed to read file %s"), quoteaf (filename));

  /* Interpret symbols.  */
  int err = tokenize_and_optimize (source, &tokenized_source, optimization_level);
  if (err != 0)
    {
      error (0, 0, _("error code: %i"), err);
      free (source);
      exit (err);
    }

  err = translate_to_asm (out_asm, &tokenized_source);
  if (err != 0)
    error (0, 0, _("error code: %i"), err);

  free (tokenized_source.tokens);
  free (source);

  if (err == 0 && do_assemble)
    {
      err = compile_to_obj (out_asm, out_obj);

      if (err == 0 && do_link)
        err = link_to_elf (out_obj, out_filename);
    }

  if (!save_temps || err != 0)
    {
      char *rm[] = { "rm", "-f", do_assemble ? out_asm : (char *) NULL, do_link ? out_obj : (char *) NULL, (char *) NULL };
      exec (rm);
    }

  free (out_filename);
  free (out_obj);
  free (out_asm);

  return err;
}

int
main (int argc, char **argv)
{
  /* Setting values of global variables.  */
  initialize_main (&argc, &argv);
  set_program_name (argv[0]);
  setlocale (LC_ALL, "");
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);

  atexit (close_stdout);

  parseopt (argc, argv);

  argc -= optind;
  argv += optind;

  int err = 0;

  for (int i = 0; i < argc; i++)
    err += compile_file (argv[i]);

  return err == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
