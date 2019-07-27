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
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <getopt.h>

#include "system.h"

#include "arch.h"
#include "compiler.h"
#include "tokenizer.h"

#include "die.h"
#include "filenamecat.h"
#include "long-options.h"
#include "tempname.h"
#include "xdectoint.h"

/* The official name of this program (e.g., no 'g' prefix).  */
#define PROGRAM_NAME "bfc"

#define AUTHORS \
  proper_name ("Sergey Sushilin")

static char *
read_file (const char *filename)
{
  char *buffer = NULL;
  size_t length = 0;
  FILE *handle = fopen (filename, "r");

  if (handle != NULL)
    {
      fseek (handle, 0L, SEEK_END);
      length = ftell (handle);
      fseek (handle, 0L, SEEK_SET);
      buffer = xmalloc (length + 1);
      size_t result = fread (buffer, sizeof (char), length, handle);
      if (result != length)
        {
          /* Error: Reading failed */
          free (buffer);
          return NULL;
        }
      buffer[length] = '\0';
      fclose (handle);
    }
  return buffer;
}

static const char *
change_extension (char *filename, const char *new_extension)
{
  if (filename == NULL || *filename == '\0')
    return "";

  size_t len = strlen (filename);
  char *tmp = filename + len - 1;

  for (; tmp >= filename; --tmp)
    {
      if (*tmp == '.')
        {
          ++tmp;
          while ((*tmp++ = *new_extension++) != '\0')
            ;
          return filename;
        }
    }

  tmp = filename + len;
  *tmp++ = '.';
  while ((*tmp++ = *new_extension++) != '\0')
    ;

  return filename;
}

static char *
cut_path (char *str)
{
  char *tmp = strchr (str, '\0');

  while (tmp >= str && *tmp != '/')
    --tmp;

  return tmp + 1;
}

static char *in_filename               = NULL;
static char *out_filename              = "a.out";
static char *out_asm                   = NULL;
static char *out_obj                   = NULL;
static bool do_assemble                = true;
static bool do_link                    = true;
static bool save_temps                 = false;
static unsigned int optimization_level = 0;

void
usage (int status)
{
  if (status != EXIT_SUCCESS)
    emit_try_help ();
  else
    {
      puts ("\
Usage: bfc [-sco:O:]\n\
  --help                   Display this information and exit.\n\
  --version                Display compiler's version and exit.\n\
  --save-temps             Do not delete temporary files.\n\
  -s                       Compile only, do not assemble or link.\n\
  -c                       Compile and assemble, but do not link.\n\
  -o <file>                Place the output into <file>.\n\
  -On                      Level of optimization, default is 0.\n");
    }

  exit (status);
}

enum
{
  SAVE_TEMPS_OPTION = CHAR_MAX + 1,
  HELP_OPTION,
  VERSION_OPTION
};

static const struct option long_options[] =
{
  {"save-temps", no_argument, NULL, SAVE_TEMPS_OPTION},
  {"help", no_argument, NULL, HELP_OPTION},
  {"version", no_argument, NULL, VERSION_OPTION},
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
  for (int i = 1; i < argc; ++i)
    {
      const char *arg = argv[i];
      if (arg[0] == '-' && strlen (arg) > 3 && lstat (arg, &st) == 0 && STRSUFFIX (arg, ".bf"))
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
    {
      switch (optc)
        {
        case 'o':
          out_filename = optarg;
          break;

        case 'O':
          optimization_level = xdectoumax (optarg, 0, 9, "", _("invalid optimization level"), 0);
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
    }

  char **operand_lim = argv + argc;
  for (char **operandp = argv + optind; operandp < operand_lim; ++operandp)
    {
      char *file = *operandp;
      if (strlen (file) > 3)
        {
          if (STRSUFFIX (file, ".bf") && in_filename == NULL)
            /* 'in_filename' by default is NULL.  */
            in_filename = file;
          else if (in_filename != NULL)
            error (0, 0, _("extra BrainFuck source code file %s"), quoteaf (file));
        }
    }

  if (in_filename == NULL)
    die (EXIT_FAILURE, 0, _("fatal error: no input files."));
}

static size_t
count_consecutive_X_s (const char *s, size_t len)
{
  size_t n = 0;
  for (; len != 0 && s[len - 1] == 'X'; --len)
    ++n;
  return n;
}
static int
mktemp_len (char *tmpl, size_t suff_len, size_t x_len, bool create)
{
  return gen_tempname_len (tmpl, suff_len, 0, create ? GT_FILE : GT_NOCREATE, x_len);
}

char *
mktmp (const char *template, size_t suff_len)
{
  char *env = getenv ("TMPDIR");
  char *tmpdir = (env != NULL && *env != '\0' ? env : "/tmp");

  char *tmpfile = file_name_concat (tmpdir, template, NULL);

  char *x_suff = strchr (template, 'X');
  if (x_suff == NULL)
    die (EXIT_TROUBLE, 0, _("invalid template %s, template must end by three or more X's"), quote (template));

  size_t x_len = count_consecutive_X_s (x_suff, strlen (x_suff) - suff_len);
  if (x_len < 3)
    die (EXIT_TROUBLE, 0, _("too few X's in template %s, template must end by three or more X's"), quote (template));

  int fd = mktemp_len (tmpfile, suff_len, x_len, true);

  if (fd < 0 || close (fd) != 0)
    error (0, errno, _("failed to create file via template %s"),
           quote (template));

  return tmpfile;
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

  char *cut_in_filename = cut_path (in_filename);
  const size_t cut_in_filename_len = strlen (cut_in_filename);

  if (save_temps || !do_link)
    {
      if (!save_temps)
        {
          if (!do_assemble)
            {
              out_asm = xstrndup (cut_in_filename, cut_in_filename_len - 1);
              change_extension (out_asm, "s");
            }
          else
            {
              out_asm = mktmp ("bfc-XXXXXXXXXXXX.s", 2);

              out_obj = xstrndup (cut_in_filename, cut_in_filename_len - 1);
              change_extension (out_obj, "o");
            }
        }
      else
        {
          out_asm = xstrndup (cut_in_filename, cut_in_filename_len - 1);
          out_obj = xstrndup (cut_in_filename, cut_in_filename_len - 1);

          change_extension (cut_in_filename, "s");
          change_extension (cut_in_filename, "o");
        }
    }
  else
    {
      out_asm = mktmp ("bfc-XXXXXXXXXXXX.s", 2);
      out_obj = mktmp ("bfc-XXXXXXXXXXXX.o", 2);
    }

  bool out_filename_was_allocated = false;

  if ((!do_assemble || !do_link) && !save_temps && !strcmp (out_filename, "a.out"))
    {
      char *buf = xmalloc (cut_in_filename_len);

      /* Write in buf filename + '.' */
      snprintf (buf, cut_in_filename_len, "%s", cut_in_filename);
      buf[cut_in_filename_len - 1] = (do_assemble ? 'o' : 's');
      buf[cut_in_filename_len - 0] = '\0';

      out_filename = buf;
      out_filename_was_allocated = true;
    }

  ProgramSource tokenized_source;
  int err = 0;

  /* Open file.  */
  char *source = read_file (in_filename);
  if (source == NULL)
    die (EXIT_FAILURE, 0, _("fatal error: failed to read file %s"), in_filename);

  /* Interpret symbols.  */
  err = tokenize_and_optimize (source, &tokenized_source, optimization_level);
  if (err != 0)
    {
      error (0, 0, _("error code: %d"), err);
      free (source);
      exit (err);
    }

  err = translate_to_asm (out_asm, &tokenized_source);
  if (err != 0)
    error (0, 0, _("error code: %d"), err);

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

  if (out_filename_was_allocated)
    free (out_filename);
  free (out_obj);
  free (out_asm);

  return err;
}
