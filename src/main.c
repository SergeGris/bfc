
#include <config.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <locale.h>
#include <stdnoreturn.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <getopt.h>

#include "system.h"

#include "tokenizer.h"
#include "compiler.h"

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
      buffer = malloc ((length + 1) * sizeof (char));
      if (buffer != NULL)
        {
          size_t result = fread (buffer, sizeof (char), length, handle);
          if (result != length)
            {
              /* Error: Reading failed */
              free (buffer);
              return NULL;
            }
          buffer[length] = '\0';
        }
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

char *
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
static bool assemble_it                = true;
static bool link_it                    = true;
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
  -s                       Compile only, do not assemble or link.\n\
  -c                       Compile and assemble, but do not link.\n\
  -o <file>                Place the output into <file>.\n\
  -On                      Level of optimization, default is 1.\n");
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

/* Advise the user about invalid usages like "bfc -foo" if the file
   "-foo" exists, assuming ARGC and ARGV are as with 'main'.  */
static void
diagnose_leading_hyphen (int argc, char **argv)
{
  /* OPTIND is unreliable, so iterate through the arguments looking
     for a file name that looks like an option.  */

  struct stat st;
  for (int i = 1; i < argc; ++i)
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
    {
      switch (optc)
        {
        case 'o':
          out_filename = optarg;
          break;

        case 'O':
          optimization_level = xdectoumax (optarg, 0, UINT8_MAX, "", _("invalid optimization level"), 0);
          if (optimization_level > 2)
            {
              /* Maximum optimization level is two, if after '-O' costs 3
                 or greater number, then replace it by 2.  */
              optimization_level = 2;
            }
          break;

        case 'c':
          assemble_it = true;
          link_it = false;
          break;

        case 's':
          assemble_it = link_it = false;
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
            {
              /* 'in_filename' by default is NULL.  */
              in_filename = file;
            }
          else if (in_filename != NULL)
            {
              error (0, 0, _("extra BrainFuck source code file %s"), quoteaf (file));
            }
        }
    }

  if (in_filename == NULL)
    {
      die (EXIT_FAILURE, 0, _("fatal error: no input files."));
    }
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
    {
      error (0, errno, _("failed to create file via template %s"),
             quote (template));
    }

  return tmpfile;
}

int
exec (char **arg)
{
  int status = -1;
  pid_t pid  = vfork ();

  if (pid < 0)
    {
      /* Fork error.  */
      die (EXIT_FAILURE, errno, "%s", arg[0]);
    }
  else if (pid == 0)
    {
      /* Child process.  */
      execvp (arg[0], arg);
    }
  else /* pid > 0 */
    {
      /* Parent process.  */
      wait (&status);
    }

  return status;
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

  if (save_temps)
    {
      out_asm = malloc (cut_in_filename_len - 1);
      out_obj = malloc (cut_in_filename_len - 1);
      snprintf (out_asm, cut_in_filename_len - 1, "%s", cut_in_filename);
      snprintf (out_obj, cut_in_filename_len - 1, "%s", cut_in_filename);
      out_asm[cut_in_filename_len - 2] = 's';
      out_asm[cut_in_filename_len - 1] = '\0';
      out_obj[cut_in_filename_len - 2] = 'o';
      out_obj[cut_in_filename_len - 1] = '\0';
    }
  else
    {
      out_obj = mktmp ("bfc-XXXXXXXXXXXX.o", 2);
      out_asm = mktmp ("bfc-XXXXXXXXXXXX.s", 2);
    }

  bool out_filename_was_allocated = false;

  if ((!assemble_it || !link_it) && !save_temps && !strcmp (out_filename, "a.out"))
    {
      char *buf = malloc (cut_in_filename_len);
      if (buf == NULL)
        die (EXIT_FAILURE, 0, "out of memory");

      /* Write in buf filename + '.' */
      snprintf (buf, cut_in_filename_len, "%s", cut_in_filename);
      buf[cut_in_filename_len - 1] = (assemble_it ? 'o' : 's');
      buf[cut_in_filename_len - 0] = '\0';

      out_filename = buf;
      out_filename_was_allocated = true;
    }

  ProgramSource tokenized_source;
  int err = 0;

  /* Open file.  */
  char *source = read_file (in_filename);
  if (source == NULL)
    {
      error (0, 0, "fatal error: failed to read file %s", in_filename);
      exit (EXIT_FAILURE);
    }

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
    {
      error (0, 0, _("error code: %d"), err);
    }

  free (tokenized_source.tokens);
  free (source);

  if (assemble_it && err == 0)
    {
      /* Run GAS to compile it if 'assemble' == true.  */
      char *as[] = { "as", "--32", "-o", out_obj, out_asm, (char *) NULL };

      err = exec (as);
      if (err != 0)
        {
          error (0, 0, "error: as returned %d exit status", err);
        }

      if (link_it && err == 0)
        {
          /* Run LD to link it if 'link' == true.  */
          char *ld[] = { "ld", "-m", "elf_i386", "-s", "-o", out_filename, out_obj, (char *) NULL };

          err = exec (ld);
          if (err != 0)
            {
              error (0, 0, "error: ld returned %d exit status", err);
            }
        }
      if (!save_temps)
        {
          /* If do not save temp files, we delete object file and
             file with assembler source code.  */
          char *rm[] = { "rm", "-f", out_asm, (char *) NULL, (char *) NULL };

          if (link_it)
            rm[3] = out_obj;

          err = exec (rm);
        }
    }

  if (out_filename_was_allocated)
    {
      free (out_filename);
    }
  free (out_obj);
  free (out_asm);

  return err;
}
