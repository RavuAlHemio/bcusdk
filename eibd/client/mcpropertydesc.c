/*
    EIBD client library
    Copyright (C) 2005-2007 Martin Kögler <mkoegler@auto.tuwien.ac.at>

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
MC_PropertyDesc_complete (EIBConnection * con)
{
  int i;
  i = _EIB_GetRequest (con);
  if (i == -1)
    return -1;
  if (EIBTYPE (con) != EIB_MC_PROP_DESC || con->size < 6)
    {
      errno = ECONNRESET;
      return -1;
    }
  /* Type */
  if (con->req.ptr2)
    *con->req.ptr2 = con->buf[2];
  /* max_nr_of_elem */
  if (con->req.ptr4)
    *con->req.ptr4 = (con->buf[3] << 8) | (con->buf[4]);
  /* access */
  if (con->req.ptr3)
    *con->req.ptr3 = con->buf[5];
  return 0;
}

int
EIB_MC_PropertyDesc_async (EIBConnection * con, uint8_t obj, uint8_t property,
			   uint8_t * type, uint16_t * max_nr_of_elem,
			   uint8_t * access)
{
  uchar head[5];
  if (!con)
    {
      errno = EINVAL;
      return -1;
    }
  con->req.ptr2 = type;
  con->req.ptr4 = max_nr_of_elem;
  con->req.ptr3 = access;
  EIBSETTYPE (head, EIB_MC_PROP_DESC);
  head[2] = obj;
  head[3] = property;
  if (_EIB_SendRequest (con, 4, head) == -1)
    return -1;
  con->complete = MC_PropertyDesc_complete;
  return 0;
}

int
EIB_MC_PropertyDesc (EIBConnection * con, uint8_t obj, uint8_t property,
		     uint8_t * type, uint16_t * max_nr_of_elem,
		     uint8_t * access)
{
  if (EIB_MC_PropertyDesc_async
      (con, obj, property, type, max_nr_of_elem, access) == -1)
    return -1;
  return EIBComplete (con);
}
