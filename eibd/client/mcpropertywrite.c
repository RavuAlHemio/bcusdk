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
MC_PropertyWrite_complete (EIBConnection * con)
{
  int i;
  i = _EIB_GetRequest (con);
  if (i == -1)
    return -1;
  if (EIBTYPE (con) != EIB_MC_PROP_WRITE)
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
EIB_MC_PropertyWrite_async (EIBConnection * con, uint8_t obj,
			    uint8_t property, uint16_t start,
			    uint8_t nr_of_elem, int len, const uint8_t * buf,
			    int max_len, uint8_t * res)
{
  uchar *ibuf;
  int i;
  if (!con)
    {
      errno = EINVAL;
      return -1;
    }
  if (!buf || !res)
    {
      errno = EINVAL;
      return -1;
    }
  con->req.len = max_len;
  con->req.buf = res;
  ibuf = (uchar *) malloc (len + 7);
  if (!ibuf)
    {
      errno = ENOMEM;
      return -1;
    }
  EIBSETTYPE (ibuf, EIB_MC_PROP_WRITE);
  ibuf[2] = obj;
  ibuf[3] = property;
  ibuf[4] = (start >> 8) & 0xff;
  ibuf[5] = (start) & 0xff;
  ibuf[6] = nr_of_elem;
  memcpy (ibuf + 7, buf, len);
  i = _EIB_SendRequest (con, len + 7, ibuf);
  free (ibuf);
  if (i == -1)
    return -1;
  con->complete = MC_PropertyWrite_complete;
  return 0;
}

int
EIB_MC_PropertyWrite (EIBConnection * con, uint8_t obj, uint8_t property,
		      uint16_t start, uint8_t nr_of_elem, int len,
		      const uint8_t * buf, int max_len, uint8_t * res)
{
  if (EIB_MC_PropertyWrite_async
      (con, obj, property, start, nr_of_elem, len, buf, max_len, res) == -1)
    return -1;
  return EIBComplete (con);
}
