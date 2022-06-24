 /*
  *----------------------------------------------------------------------------
  *"THE BEER-WARE LICENSE" (Revision 42):
  *<gustek@riseup.net> wrote this file.  As long as you retain this notice you
  *can do whatever you want with this stuff. If we meet some day, and you think
  *this stuff is worth it, you can buy me a beer in return.
  *----------------------------------------------------------------------------
  */
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_LINE (4096)
#define NOTIFICATION_SIZE (sizeof(struct inotify_event) + NAME_MAX + 1)
#define BUFFER_SIZE (1024 * (NOTIFICATION_SIZE))
#define progname() ("woyf")

static int inotify_fd;

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
  char buffer[MAX_LINE];

  while (fgets (buffer, MAX_LINE, stdin) != NULL)
    {
      if (inotify_add_watch (inotify_fd, trim(buffer), IN_CREATE | IN_DELETE | IN_DELETE_SELF | IN_MODIFY) == -1)
	return -1;
    }

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

static void
exit_cleanly (sig)
     int sig;
{
  (void)sig;
  close (inotify_fd);
  fprintf (stderr, "%s: exiting cleanly. Goodbye.\n", progname());

  exit (EXIT_SUCCESS);
}

int
main (argc, argv)
     int argc;
     char *const argv[];
{
  pid_t pid;
  struct sigaction action;
  uint8_t buffer[BUFFER_SIZE];

  if (argc < 2)
    {
      fprintf (stderr, "%s: no command supplied\n", progname());
      return EXIT_FAILURE;
    }

  memset (&action, 0, sizeof(action));
  action.sa_handler = &exit_cleanly;

  if (sigaction (SIGTERM, &action, NULL) == -1)
    return EXIT_FAILURE;

  inotify_fd = inotify_init ();
  if (inotify_fd == -1)
    {
      fprintf (stderr, "%s: failed to initialize an inotify instance\n", progname());

      return EXIT_FAILURE;
    }

  if (getfilelist () != 0)
    {
      fprintf (stderr, "%s: failed to add files to the watchlist\n", progname());
      close (inotify_fd);

      return EXIT_FAILURE;
    }

  pid = fork ();

  if (pid < 0)
    {
      fprintf (stderr, "%s: failed to fork process\n", progname());
      close (inotify_fd);

      return EXIT_FAILURE;
    }

  if (pid > 0)
    {
      fprintf (stderr, "%s: process id %d\n", progname(), pid);
      exit (EXIT_SUCCESS);
    }

  for (;;)
    {
      int ret, bytes, i;

      if (bytes = read (inotify_fd, buffer, BUFFER_SIZE), bytes == -1)
	{
	  fprintf (stderr, "%s: failed to read filesystem event\n", progname());
	  close (inotify_fd);

	  return EXIT_FAILURE;
	}

      for (i = 0; i < bytes; i += NOTIFICATION_SIZE)
	{
	  if (ret = exec_command (argv + 1), ret != 0)
	    fprintf (stderr, "%s: command execution failed with exit code %d\n", progname(), ret);
	}
    }
}

