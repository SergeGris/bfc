
#include <config.h>

#include "compiler.h"

#include <assert.h>
#include <error.h>
#include <stdio.h>
#include <stdlib.h>

#include "system.h"

#include "arch.h"

int
str_append (char **str, const char *format, ...)
{
  /* This is only used to combine arguments,
     so fixed-size string should be safe to use.  */
  char formatted_str[512];
  assert (strlen (format) <= sizeof (formatted_str));

  va_list arg_ptr;
  va_start (arg_ptr, format);

  vsprintf (formatted_str, format, arg_ptr);

  const size_t old_length = (*str == NULL ? 0 : strlen (*str));
  char *new_str = calloc (old_length + strlen (formatted_str) + 1, sizeof (char));
  if (new_str == NULL)
    {
      /* Error: Out of memory.  */
      error (0, 0, "out of memory");
      return -1;
    }

  if (*str != NULL)
    strcat (new_str, *str);
  strcat (new_str, formatted_str);

  free (*str);
  *str = new_str;

  return 0;
}

static int
write_file (const char *filename,
            const char *source,
            const size_t source_length)
{
  FILE *handle = fopen (filename, "w");
  if (handle == NULL)
    {
      /* Error: Writing failed.  */
      error (0, errno, "%s", filename);
      return -1;
    }
  size_t result = fwrite (source, sizeof (char), source_length, handle);
  if (result != source_length)
    {
      /* Error: Writing failed.  */
      error (0, errno, "%s", filename);
      return -1;
    }
  fclose (handle);
  return 0;
}

int
translate_to_asm (const char *filename,
                  ProgramSource *const source)
{
  char *instructions = NULL;
  size_t instructions_length = 0;
  int err = tokens_to_asm (source, &instructions, &instructions_length);
  if (err != 0)
    return err;
  err = write_file (filename, instructions, instructions_length);
  free (instructions);

  return err;
}
