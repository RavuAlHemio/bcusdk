/*
    EIB Demo program - common functions
    Copyright (C) 2005 Martin Kögler <mkoegler@auto.tuwien.ac.at>

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
#include "common.h"
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

void
printHex (int len, uchar * data)
{
  int i;
  for (i = 0; i < len; i++)
    printf ("%02X ", data[i]);
}

void
die (const char *msg, ...)
{
  va_list ap;
  va_start (ap, msg);
  vprintf (msg, ap);
  va_end (ap);
  printf (": %s\n", strerror (errno));
  exit (1);
}

eibaddr_t
readaddr (const char *addr)
{
  int a, b, c;
  sscanf (addr, "%d.%d.%d", &a, &b, &c);
  return ((a & 0x0f) << 12) | ((b & 0x0f) << 8) | ((c & 0xff));
}

eibaddr_t
readgaddr (const char *addr)
{
  int a, b, c;
  sscanf (addr, "%d/%d/%d", &a, &b, &c);
  return ((a & 0x0f) << 12) | ((b & 0x0f) << 8) | ((c & 0xff));
}

unsigned
readHex (const char *addr)
{
  int i;
  sscanf (addr, "%x", &i);
  return i;
}

int
readBlock (uchar * buf, int size, int ac, char *ag[])
{
  int i = 0;
  while (size - i > 0 && ac > 0)
    {
      buf[i] = readHex (ag[i]);
      i++;
      ac--;
    }
  return i;
}
