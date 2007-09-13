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

int
EIBGetBusmonitorPacket (EIBConnection * con, int maxlen, uint8_t * buf)
{
  int i;
  if (!con || !buf)
    {
      errno = EINVAL;
      return -1;
    }

  i = _EIB_GetRequest (con);
  if (i == -1)
    return -1;

  if (EIBTYPE (con) != EIB_BUSMONITOR_PACKET)
    {
      errno = ECONNRESET;
      return -1;
    }
  i = con->size - 2;
  if (i > maxlen)
    i = maxlen;
  memcpy (buf, con->buf + 2, i);
  return i;
}
