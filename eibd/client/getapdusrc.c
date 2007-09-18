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
EIBGetAPDU_Src_complete (EIBConnection * con)
{
  EIBC_GETREQUEST
  EIBC_CHECKRESULT (EIB_APDU_PACKET, 4)
  if (con->req.ptr5)
    *con->req.ptr5 = (con->buf[2] << 8) | (con->buf[3]);
  EIBC_RETURN_BUF (4)
}

int
EIBGetAPDU_Src_async (EIBConnection * con, int maxlen, uint8_t * buf,
		      eibaddr_t * src)
{
  if (!con || !buf)
    {
      errno = EINVAL;
      return -1;
    }

  con->req.buf = buf;
  con->req.len = maxlen;
  con->req.ptr5 = src;
  con->complete = EIBGetAPDU_Src_complete;
  return 0;
}

int
EIBGetAPDU_Src (EIBConnection * con, int maxlen, uint8_t * buf,
		eibaddr_t * src)
{
  if (EIBGetAPDU_Src_async (con, maxlen, buf, src) == -1)
    return -1;
  return EIBComplete (con);
}
