
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
#include "long-options.h"
#include "xdectoint.h"

/* The official name of this program (e.g., no 'g' prefix).  */
#define PROGRAM_NAME "bfc"

#define AUTHORS \
  proper_name ("Sergey Sushilin")

char *
read_file (const char *filename)
{
  char *buffer = NULL;
  size_t length = 0;
  FILE *handle = fopen (filename, "rb");

  if (handle != NULL)
    {
      fseek (handle, 0, SEEK_END);
      length = ftell (handle);
      fseek (handle, 0, SEEK_SET);
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

const char *
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

static char *in_filename = NULL;
static char *out_filename = "a.out";
static bool assemble_it = true;
static bool link_it = true;
static bool save_temps = false;
static int  optimization_level = 1;

void __attribute__((__noreturn__))
usage (int status)
{
  if (status != EXIT_SUCCESS)
		emit_try_help ();
  else
    {
      puts ("Usage: bfc [-sco:O:]\n"
						"  --help                   Display this information and exit.\n"
						"  -s                       Compile only, do not assemble or link.\n"
						"  -c                       Compile and assemble, but do not link.\n"
						"  -o <file>                Place the output into <file>.\n"
						"  -On                      Level of optimization, default is 1.\n");
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

  int opt = -1;

  while ((opt = getopt_long (argc, argv, "o:O:cs", long_options, NULL)) != -1)
	  {
			switch (opt)
				{
				case 'o':
					out_filename = optarg;
					break;

				case 'O':
					optimization_level = xdectoumax (optarg, 0, UINT_MAX, "", _("invalid optimization level"), 0);
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
					/* BrainFuck source code files must have '.bf' extension.  */
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
			die (EXIT_FAILURE, 0, _("\
\x1b[31mfatal error:\x1b[0m no input files\n\
compilation terminated."));
		}
}

int
exec (char **arg)
{
  int status;
	pid_t pid = -1;

  if ((pid = fork ()) < 0)
    {
      /* Fork error.  */
      die (EXIT_FAILURE, errno, "%s", program_name);
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

  bool out_filename_was_allocated = false;
  if ((!assemble_it || !link_it) && !strcmp (out_filename, "a.out"))
    {
      char *cut_in_filename = cut_path (in_filename);
      const size_t len = strlen (cut_in_filename) - 1;
      char *buf = malloc (len); /* Allocate one less byte, '.bf' -> '.s' or '.o'.  */

			/* Write in buf filename + '.' */
      snprintf (buf, len, cut_in_filename);
      buf[len - 1] = (assemble_it ? 'o' : 's');
      buf[len] = '\0';

      out_filename = buf;
      out_filename_was_allocated = true;
    }

  ProgramSource tokenized_source;
  int err = 0;

  /* Open file.  */
  char *source = read_file (in_filename);
  if (source == NULL)
    {
      printf ("bfc: \x1b[31mfatal error:\x1b[0m failed to read file %s\n",
              in_filename);
      exit (EXIT_FAILURE);
    }

  /* Interpret symbols.  */
  err = tokenize_and_optimize (source, &tokenized_source, optimization_level);
  if (err != 0)
    {
      printf ("bfc: \x1b[31merror code:\x1b[0m %d\n", err);
      free (source);
      exit (err);
    }

  /* Write result file.  */
  err = compile_to_file (out_filename, FILETYPE_ASSEMBLY, &tokenized_source);
  if (err != 0)
    {
      printf ("bfc: \x1b[31merror code:\x1b[0m %d\n", err);
    }

  free (tokenized_source.tokens);
  free (source);

  char *out_obj = malloc (strlen (out_filename) + 1); /* Reserve 1 byte for '\0'.  */
  strcpy (out_obj, out_filename);
  change_extension (out_obj, "o");

  if (assemble_it)
    {
      /* Run GAS to compile it if 'assemble' == true.  */
      char *as[] = { "as", "--32", "-o", out_obj, out_filename, (char *) NULL };

      exec (as);

      if (link_it)
        {
          /* Run LD to link it if 'link' == true.  */
          char *ld[] = { "ld", "-m", "elf_i386", "-s", "-o", out_filename, out_obj, (char *) NULL };

					exec (ld);
        }
      if (!save_temps)
        {
          /* If do not linking file and do not save temp files,
             we delete file with assembler source code.  */
          char *rm[] = { "rm", "-f", out_obj, (char *) NULL };

          exec (rm);
        }
    }

  free (out_obj);
  if (out_filename_was_allocated)
    {
      free (out_filename);
    }

  return err;
}
