/*
    EIB Demo program - read device memory
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

int
main (int ac, char *ag[])
{
  int len, addr;
  EIBConnection *con;
  uchar buf[255];
  eibaddr_t dest;

  if (ac != 5)
    die ("usage: %s url eibaddr addr count", ag[0]);
  con = EIBSocketURL (ag[1]);
  if (!con)
    die ("Open failed");
  dest = readaddr (ag[2]);
  addr = readHex (ag[3]);
  len = atoi (ag[4]);

  if (EIB_MC_Connect (con, dest) == -1)
    die ("Connect failed");

  len = EIB_MC_Read (con, addr, len, buf);
  if (len == -1)
    die ("Read failed");
  printHex (len, buf);

  EIBClose (con);
  return 0;
}
