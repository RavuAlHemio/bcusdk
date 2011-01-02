/*
    EIB Demo program - describe property
    Copyright (C) 2005-2011 Martin Koegler <mkoegler@auto.tuwien.ac.at>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#include "common.h"

int
main (int ac, char *ag[])
{
  int len, obj, prop;
  EIBConnection *con;
  eibaddr_t dest;
  uchar type, access;
  uint16_t count;
  char *prog = ag[0];

  parseKey (&ac, &ag);
  if (ac != 5)
    die ("usage: %s [-k key] url eibaddr obj prop", prog);
  con = EIBSocketURL (ag[1]);
  if (!con)
    die ("Open failed");
  dest = readaddr (ag[2]);
  obj = atoi (ag[3]);
  prop = atoi (ag[4]);

  if (EIB_MC_Connect (con, dest) == -1)
    die ("Connect failed");
  auth (con);

  len = EIB_MC_PropertyDesc (con, obj, prop, &type, &count, &access);
  if (len == -1)
    die ("Read failed");
  printf ("Property: type:%d count:%d access:%02X\n", type, count, access);

  EIBClose (con);
  return 0;
}
