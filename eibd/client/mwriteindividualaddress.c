/*
    EIBD client library
    Copyright (C) 2005-2007 Martin Koegler <mkoegler@auto.tuwien.ac.at>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    In addition to the permissions in the GNU General Public License, 
    you may link the compiled version of this file into combinations
    with other programs, and distribute those combinations without any 
    restriction coming from the use of this file. (The General Public 
    License restrictions do apply in other respects; for example, they 
    cover modification of the file, and distribution when not linked into 
    a combine executable.)

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "eibclient.h"
#include "eibclient-int.h"

static int
M_WriteIndividualAddress_complete (EIBConnection * con)
{
  int i;
  i = _EIB_GetRequest (con);
  if (i == -1)
    return -1;
  if (EIBTYPE (con) == EIB_ERROR_ADDR_EXISTS)
    {
      errno = EADDRINUSE;
      return -1;
    }
  if (EIBTYPE (con) == EIB_M_INDIVIDUAL_ADDRESS_WRITE)
    {
      return 0;
    }
  if (EIBTYPE (con) == EIB_ERROR_TIMEOUT)
    {
      errno = ETIMEDOUT;
      return -1;
    }
  if (EIBTYPE (con) == EIB_ERROR_MORE_DEVICE)
    {
      errno = EADDRNOTAVAIL;
      return -1;
    }
  errno = ECONNRESET;
  return -1;
}

int
EIB_M_WriteIndividualAddress_async (EIBConnection * con, eibaddr_t dest)
{
  uchar head[4];
  if (!con)
    {
      errno = EINVAL;
      return -1;
    }
  EIBSETTYPE (head, EIB_M_INDIVIDUAL_ADDRESS_WRITE);
  EIBSETADDR (head + 2, dest);
  if (_EIB_SendRequest (con, 4, head) == -1)
    return -1;
  con->complete = M_WriteIndividualAddress_complete;
  return 0;
}

int
EIB_M_WriteIndividualAddress (EIBConnection * con, eibaddr_t dest)
{
  if (EIB_M_WriteIndividualAddress_async (con, dest) == -1)
    return -1;
  return EIBComplete (con);
}
