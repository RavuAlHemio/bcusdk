#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <argp.h>
#include "addrtab.h"
#include "lowlevelconf.h"

/** aborts program with a printf like message */
void
die (const char *msg, ...)
{
  va_list ap;
  va_start (ap, msg);
  vprintf (msg, ap);
  printf ("\n");
  va_end (ap);

  exit (1);
}

/** structure to store low level backends */
struct urldef
{
  /** URL-prefix */
  const char *prefix;
  /** factory function */
  LowLevel_Create_Func Create;
};

/** list of URLs */
struct urldef URLs[] = {
#undef L2_NAME
#define L2_NAME(a) { a##_PREFIX, a##_CREATE },
#include "lowlevelcreate.h"
  {0, 0}
};

/** determines the right backend for the url and creates it */
LowLevelDriverInterface *
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

/** version */
const char *argp_program_version = "bcuaddrtab " VERSION;
/** documentation */
static char doc[] =
  "bcuaddrtab -- read BCU address table size (or write it with -w)\n"
  "(C) 2005 Martin K�gler <mkoegler@auto.tuwien.ac.at>\n"
  "supported URLs are:\n"
#undef L2_NAME
#define L2_NAME(a) a##_URL
#include "lowlevelcreate.h"
  "\n"
#undef L2_NAME
#define L2_NAME(a) a##_DOC
#include "lowlevelcreate.h"
  "\n";

/** structure to store the arguments */
struct arguments
{
  /** trace level */
  int tracelevel;
  /** length to write */
  int newlength;
};
/** storage for the arguments*/
struct arguments arg;



/** documentation for arguments*/
static char args_doc[] = "URL";

/** option list */
static struct argp_option options[] = {

  {"trace", 't', "LEVEL", 0, "set trace level"},
  {"write", 'w', "LEVEL", 0, "value to write"},
  {0}
};


/** parses and stores an option */
static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
  struct arguments *arguments = (struct arguments *) state->input;
  switch (key)
    {
    case 't':
      arguments->tracelevel = (arg ? atoi (arg) : 0);
      break;
    case 'w':
      arguments->newlength = (arg ? atoi (arg) : 0);
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
  LowLevelDriverInterface *iface = 0;
  memset (&arg, 0, sizeof (arg));
  arg.newlength = -1;

  argp_parse (&argp, ac, ag, 0, &index, &arg);
  if (index > ac - 1)
    die ("url expected");
  if (index < ac - 1)
    die ("unexpected parameter");

  signal (SIGPIPE, SIG_IGN);
  pth_init ();

  Trace t;
  t.SetTraceLevel (arg.tracelevel);

  try
  {
    iface = Create (ag[index], &t);
  }
  catch (Exception e)
  {
    die ("initialisation failed");
  }

  uchar res = arg.newlength;
  if (arg.newlength == -1)
    {
      if (!readAddrTabSize (iface, res))
	die ("read failed");
      printf ("Size: %d\n", res);
    }
  else if (arg.newlength >= 0 && arg.newlength <= 0xff)
    {
      if (!writeAddrTabSize (iface, res))
	die ("write failed");
      printf ("Size %d written\n", res);
    }
  else
    die ("invalid value %d to write", arg.newlength);

  delete iface;

  pth_exit (0);
  return 0;
}