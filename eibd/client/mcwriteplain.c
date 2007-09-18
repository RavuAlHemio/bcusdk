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
MC_Write_Plain_complete (EIBConnection * con)
{
  EIBC_GETREQUEST
  EIBC_CHECKRESULT (EIB_MC_WRITE_NOVERIFY, 2)
  EIBC_RETURN_LEN
}

int
EIB_MC_Write_Plain_async (EIBConnection * con, uint16_t addr, int len,
			  const uint8_t * buf)
{
  uchar *ibuf;
  int i;
  if (!con)
    {
      errno = EINVAL;
      return -1;
    }
  con->req.len = len;
  if (!buf)
    {
      errno = EINVAL;
      return -1;
    }
  ibuf = (uchar *) malloc (len + 6);
  if (!ibuf)
    {
      errno = ENOMEM;
      return -1;
    }
  EIBSETTYPE (ibuf, EIB_MC_WRITE_NOVERIFY);
  ibuf[2] = (addr >> 8) & 0xff;
  ibuf[3] = (addr) & 0xff;
  ibuf[4] = (len >> 8) & 0xff;
  ibuf[5] = (len) & 0xff;
  memcpy (ibuf + 6, buf, len);
  i = _EIB_SendRequest (con, len + 6, ibuf);
  free (ibuf);
  if (i == -1)
    return -1;
  con->complete = MC_Write_Plain_complete;
  return 0;
}

int
EIB_MC_Write_Plain (EIBConnection * con, uint16_t addr, int len,
		    const uint8_t * buf)
{
  if (EIB_MC_Write_Plain_async (con, addr, len, buf) == -1)
    return -1;
  return EIBComplete (con);
}
