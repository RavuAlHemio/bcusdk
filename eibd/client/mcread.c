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

EIBC_COMPLETE (EIB_MC_Read,
  EIBC_GETREQUEST
  EIBC_CHECKRESULT (EIB_MC_READ, 2)
  EIBC_RETURN_BUF (2)
)

int
EIB_MC_Read_async (EIBConnection * con, uint16_t addr, int len, uint8_t * buf)
{
  uchar head[6];
  uchar *ibuf = head;
  if (!con)
    {
      errno = EINVAL;
      return -1;
    }
  if (!buf)
    {
      errno = EINVAL;
      return -1;
    }
  con->req.len = len;
  con->req.buf = buf;
  EIBSETTYPE (ibuf, EIB_MC_READ);
  ibuf[2] = (addr >> 8) & 0xff;
  ibuf[3] = (addr) & 0xff;
  ibuf[4] = (len >> 8) & 0xff;
  ibuf[5] = (len) & 0xff;
  if (_EIB_SendRequest (con, 6, ibuf) == -1)
    return -1;
  con->complete = EIB_MC_Read_complete;
  return 0;
}

int
EIB_MC_Read (EIBConnection * con, uint16_t addr, int len, uint8_t * buf)
{
  if (EIB_MC_Read_async (con, addr, len, buf) == -1)
    return -1;
  return EIBComplete (con);
}
