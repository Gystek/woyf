 /*
  *----------------------------------------------------------------------------
  *"THE BEER-WARE LICENSE" (Revision 42):
  *<gustek@riseup.net> wrote this file.  As long as you retain this notice you
  *can do whatever you want with this stuff. If we meet some day, and you think
  *this stuff is worth it, you can buy me a beer in return.
  *----------------------------------------------------------------------------
  */
#include <errno.h>
#include <getopt.h>
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define MAX_LINE (4096)
#define progname() ("woyf")

static char **fl = NULL;
static time_t *tl = NULL;
size_t fc = 0;

static int
getfiledate (dest, path)
     time_t     *dest;
     const char *path;
{
  struct stat buf;
  if (stat (path, &buf) != 0)
    {
      fprintf (stderr, "%s: %s: %s\n", progname(), path, strerror (errno));
      return 1;
    }
  *dest = (buf.st_mtime);

  return 0;
}

static void
freefilelist (void)
{
  size_t i = 0;

  for (; i < fc; i++)
    free (fl[i]);
  free (fl);
}

static char *
trim (buffer)
     char *buffer;
{
  size_t l = strlen (buffer);
  if (buffer[0] == '\n')
    return (trim (buffer + 1));
  else if (buffer[l - 1] == '\n')
    {
      buffer[l - 1] = 0;
      return (trim (buffer));
    }

  return buffer;
}

static int
getfilelist (void)
{
  size_t i = 0;
  char buffer[MAX_LINE];

  while (fgets (buffer, MAX_LINE, stdin) != NULL)
    {

      fl = realloc (fl, sizeof(uintptr_t) * ++i);
      if (fl == NULL)
	return 1;

      fl[i - 1] = malloc (sizeof(char) * MAX_LINE);
      if (fl[i - 1] == NULL)
	{
	  freefilelist ();
	  return 1;
	}
      strcpy (fl[i - 1], trim (buffer));
    }

  fc = i;

  return 0;
}

static int
exec_command (command)
     char *const command[];
{
  pid_t pid = 0;
  int ret = 0;

  pid  = fork ();
  if (pid < 0)
    return 1;

  if (pid == 0)
    {
      ret = execvp (command[0], command);
      exit (0);
    }

  return ret;
}

static int
runturn (command)
     char *const command[];
{
  size_t i;

  for (i = 0; i < fc; i++)
    {
      const char *fname = fl[i];
      time_t current = tl[i];
      time_t new;

      if (getfiledate (&new, fname) != 0)
	return 1;

      if (new > current)
	{
	  fprintf (stderr, "%s: file `%s' has changed\n", progname(), fname);
	  tl[i] = new;
	  if (exec_command (command) != 0)
	    return 1;
	  return 0;
	}
    }

  return 0;
}

static void
exit_cleanly (sig)
     int sig;
{
  (void)sig;
  freefilelist ();
  free (tl);
  fprintf (stderr, "%s: exiting cleanly. Goodbye.\n", progname());

  exit (EXIT_SUCCESS);
}

int
main (argc, argv)
     int argc;
     char *const argv[];
{
  time_t current;
  size_t i;
  pid_t pid;
  struct sigaction action;

  if (argc < 2)
    {
      fprintf (stderr, "%s: no command supplied\n", progname());
      return EXIT_FAILURE;
    }

  memset (&action, 0, sizeof(action));
  action.sa_handler = &exit_cleanly;

  if (sigaction (SIGTERM, &action, NULL) == -1)
    return EXIT_FAILURE;

  time (&current);

  getfilelist ();
  if (fc == 0)
    return EXIT_FAILURE;

  tl = calloc (fc, sizeof(time_t));
  if (tl == NULL)
    {
      freefilelist ();
      return EXIT_FAILURE;
    }

  for (i = 0; i < fc ; i++)
    tl[i] = current;

  pid = fork ();

  if (pid < 0)
    {
      fprintf (stderr, "%s: failed to fork process\n", progname());
    }

  if (pid > 0)
    {
      fprintf (stderr, "%s: process id %d\n", progname(), pid);
      exit (EXIT_SUCCESS);
    }

  for (;;)
    {
      runturn (argv + 1);
      sleep(1);
    }
}
