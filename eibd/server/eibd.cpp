/*
    EIBD eib bus access and management daemon
    Copyright (C) 2005 Martin K�gler <mkoegler@auto.tuwien.ac.at>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <argp.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include "layer3.h"
#include "localserver.h"
#include "inetserver.h"

/** structure to store the arguments */
struct arguments
{
  /** port to listen */
  int port;
  /** path for unix domain socket */
  const char *name;
  /** path to pid file */
  const char *pidfile;
  /** path to trace log file */
  const char *daemon;
  /** trace level */
  int tracelevel;
  /** EIB address (for some backends) */
  eibaddr_t addr;
};
/** storage for the arguments*/
struct arguments arg;

/** aborts program with a printf like message */
void
die (const char *msg, ...)
{
  va_list ap;
  va_start (ap, msg);
  vprintf (msg, ap);
  printf ("\n");
  va_end (ap);

  if (arg.pidfile)
    unlink (arg.pidfile);

  exit (1);
}


#include "layer2conf.h"

/** structure to store layer 2 backends */
struct urldef
{
  /** URL-prefix */
  const char *prefix;
  /** factory function */
  Layer2_Create_Func Create;
};

/** list of URLs */
struct urldef URLs[] = {
#undef L2_NAME
#define L2_NAME(a) { a##_PREFIX, a##_CREATE },
#include "layer2create.h"
  {0, 0}
};

/** determines the right backend for the url and creates it */
Layer2Interface *
Create (const char *url, Trace * t)
{
  unsigned int p = 0;
  struct urldef *u = URLs;
  while (url[p] && url[p] != ':')
    p++;
  if (url[p] != ':')
    die ("not a valid url");
  while (u->prefix)
    {
      if (strlen (u->prefix) == p && !memcmp (u->prefix, url, p))
	{
	  return u->Create (url + p + 1, t);
	}
      u++;
    }
  die ("url not supported");
  return 0;
}

/** parses an EIB individual address */
eibaddr_t
readaddr (const char *addr)
{
  int a, b, c;
  sscanf (addr, "%d.%d.%d", &a, &b, &c);
  return ((a & 0x0f) << 12) | ((b & 0x0f) << 8) | ((c & 0xff));
}

/** version */
const char *argp_program_version = "eibd " VERSION;
/** documentation */
static char doc[] =
  "eibd -- a commonication stack for EIB\n"
  "(C) 2005 Martin K�gler <mkoegler@auto.tuwien.ac.at>\n"
  "supported urls are:\n"
#undef L2_NAME
#define L2_NAME(a) a##_URL
#include "layer2create.h"
  "\n"
#undef L2_NAME
#define L2_NAME(a) a##_DOC
#include "layer2create.h"
  "\n";

/** documentation for arguments*/
static char args_doc[] = "url";

/** option list */
static struct argp_option options[] = {
  {"listen-tcp", 'i', "PORT", OPTION_ARG_OPTIONAL,
   "listen at TCP port PORT (default 6720)"},
  {"listen-local", 'u', "FILE", OPTION_ARG_OPTIONAL,
   "listen at unix domain socket FILE (default /tmp/eib)"},
  {"trace", 't', "LEVEL", 0, "set trace level"},
  {"eibaddr", 'e', "EIBADDR", 0,
   "sets our own EIB-address to EIBADDR (default 0.0.1), for drivers, which need one address"},
  {"pid-file", 'p', "FILE", 0, "writes the PID of the process to FILE"},
  {"daemon", 'd', "FILE", OPTION_ARG_OPTIONAL,
   "starts the programm as daemon, the output will be written to FILE, if present"},
  {0}
};

/** parses and stores an option */
static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
  struct arguments *arguments = (struct arguments *) state->input;
  switch (key)
    {
    case 'u':
      arguments->name = (char *) (arg ? arg : "/tmp/eib");
      break;
    case 'i':
      arguments->port = (arg ? atoi (arg) : 6720);
      break;
    case 't':
      arguments->tracelevel = (arg ? atoi (arg) : 0);
      break;
    case 'e':
      arguments->addr = readaddr (arg);
      break;
    case 'p':
      arguments->pidfile = arg;
      break;
    case 'd':
      arguments->daemon = (char *) (arg ? arg : "/dev/null");
      break;
    default:
      return ARGP_ERR_UNKNOWN;
    }
  return 0;
}

/** information for the argument parser*/
static struct argp argp = { options, parse_opt, args_doc, doc };


int
main (int ac, char *ag[])
{
  int index;
  Queue < Server * >server;
  Layer2Interface *l2;
  Layer3 *l3;

  memset (&arg, 0, sizeof (arg));
  arg.addr = 0x0001;

  argp_parse (&argp, ac, ag, 0, &index, &arg);
  if (index > ac - 1)
    die ("url expected");
  if (index < ac - 1)
    die ("unexpected parameter");

  if (arg.port == 0 && arg.name == 0)
    die ("No listen-address given");

  signal (SIGPIPE, SIG_IGN);
  pth_init ();

  Trace t;
  t.SetTraceLevel (arg.tracelevel);

  if (arg.daemon)
    {
      int fd = open (arg.daemon, O_WRONLY | O_APPEND | O_CREAT);
      if (fd == -1)
	die ("Can not open file %s", arg.daemon);
      int i = fork ();
      if (i < 0)
	die ("fork failed");
      if (i > 0)
	exit (0);
      close (1);
      close (2);
      close (0);
      dup2 (fd, 1);
      dup2 (fd, 2);
      setsid ();
    }


  FILE *pidf;
  if (arg.pidfile)
    if ((pidf = fopen (arg.pidfile, "w")) != NULL)
      {
	fprintf (pidf, "%d", getpid ());
	fclose (pidf);
      }

  try
  {
    l2 = Create (ag[index], &t);
    l3 = new Layer3 (l2, &t);
    if (arg.port)
      server.put (new InetServer (l3, &t, arg.port));
    if (arg.name)
      server.put (new LocalServer (l3, &t, arg.name));
  }
  catch (Exception e)
  {
    die ("initialisation failed");
  }

  sigset_t t1;
  sigfillset (&t1);
  sigaddset (&t1, SIGINT);
  sigaddset (&t1, SIGTERM);

  int x;
  pth_sigwait (&t1, &x);

  while (!server.isempty ())
    delete server.get ();

  delete l3;

  if (arg.pidfile)
    unlink (arg.pidfile);

  pth_exit (0);
  return 0;
}