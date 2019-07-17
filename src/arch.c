
int exec (char **arg);

#if defined(__x86_64__) || defined(_AMD64_) || defined(__i386__)
# include "arch/x86.c"
#elif defined(__arm__)
# include "arch/arm.c"
#elif defined(__aarch64__)
# include "arch/arm64.c"
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
