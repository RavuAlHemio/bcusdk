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
MC_PropertyRead_complete (EIBConnection * con)
{
  int i;
  i = _EIB_GetRequest (con);
  if (i == -1)
    return -1;
  if (EIBTYPE (con) != EIB_MC_PROP_READ)
    {
      errno = ECONNRESET;
      return -1;
    }
  i = con->size - 2;
  if (i > con->req.len)
    i = con->req.len;
  memcpy (con->req.buf, con->buf + 2, i);
  return i;
}

int
EIB_MC_PropertyRead_async (EIBConnection * con, uint8_t obj, uint8_t property,
			   uint16_t start, uint8_t nr_of_elem, int max_len,
			   uint8_t * buf)
{
  uchar head[7];
  if (!con)
    {
      errno = EINVAL;
      return -1;
    }
  con->req.buf = buf;
  con->req.len = max_len;
  if (!buf)
    {
      errno = EINVAL;
      return -1;
    }
  EIBSETTYPE (head, EIB_MC_PROP_READ);
  head[2] = obj;
  head[3] = property;
  head[4] = (start >> 8) & 0xff;
  head[5] = (start) & 0xff;
  head[6] = nr_of_elem;
  if (_EIB_SendRequest (con, 7, head) == -1)
    return -1;
  con->complete = MC_PropertyRead_complete;
  return 0;
}

int
EIB_MC_PropertyRead (EIBConnection * con, uint8_t obj, uint8_t property,
		     uint16_t start, uint8_t nr_of_elem, int max_len,
		     uint8_t * buf)
{
  if (EIB_MC_PropertyRead_async
      (con, obj, property, start, nr_of_elem, max_len, buf) == -1)
    return -1;
  return EIBComplete (con);
}
