/*
    EIBD eib bus access and management daemon
    Copyright (C) 2005 Martin K�gler <mkoegler@auto.tuwien.ac.at>

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

#include "emi.h"

CArray
L_Data_ToCEMI (uchar code, const L_Data_PDU & l1)
{
  uchar c;
  CArray pdu;
  assert (l1.data () >= 1);
  assert (l1.data () < 0xff);
  assert ((l1.hopcount & 0xf8) == 0);

  switch (l1.prio)
    {
    case PRIO_LOW:
      c = 0x3;
      break;
    case PRIO_NORMAL:
      c = 0x1;
      break;
    case PRIO_URGENT:
      c = 0x02;
      break;
    case PRIO_SYSTEM:
      c = 0x00;
      break;
    }
  pdu.resize (l1.data () + 9);
  pdu[0] = code;
  pdu[1] = 0x00;
  pdu[2] = 0x30 | (c << 2) | (l1.data () - 1 <= 0x0f ? 0x80 : 0x00);
  pdu[3] =
    (l1.AddrType ==
     GroupAddress ? 0x80 : 0x00) | ((l1.hopcount & 0x7) << 4) | 0x0;
  pdu[4] = (l1.source >> 8) & 0xff;
  pdu[5] = (l1.source) & 0xff;
  pdu[6] = (l1.dest >> 8) & 0xff;
  pdu[7] = (l1.dest) & 0xff;
  pdu[8] = l1.data () - 1;
  pdu.setpart (l1.data.array (), 9, l1.data ());
  return pdu;
}

L_Data_PDU *
CEMI_to_L_Data (const CArray & data)
{
  L_Data_PDU c;
  if (data () < 2)
    return 0;
  unsigned start = data[1] + 2;
  if (data () < 7 + start)
    return 0;
  if (data () < 7 + start + data[6 + start] + 1)
    return 0;
  c.source = (data[start + 2] << 8) | (data[start + 3]);
  c.dest = (data[start + 4] << 8) | (data[start + 5]);
  c.data.set (data.array () + start + 7, data[6 + start] + 1);
  c.repeated = (data[start] & 0x20) ? 0 : 1;
  switch ((data[start] >> 2) & 0x3)
    {
    case 0:
      c.prio = PRIO_SYSTEM;
      break;
    case 1:
      c.prio = PRIO_URGENT;
      break;
    case 2:
      c.prio = PRIO_NORMAL;
      break;
    case 3:
      c.prio = PRIO_LOW;
      break;
    }
  c.hopcount = (data[start + 1] >> 4) & 0x07;
  c.AddrType = (data[start + 1] & 0x80) ? GroupAddress : IndividualAddress;
  if (!data[start] & 0x80 && data[start + 1] & 0x0f)
    return 0;
  return new L_Data_PDU (c);
}

