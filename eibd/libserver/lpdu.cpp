/*
    EIBD eib bus access and management daemon
    Copyright (C) 2005-2008 Martin Koegler <mkoegler@auto.tuwien.ac.at>

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

#include <stdio.h>
#include "lpdu.h"
#include "tpdu.h"

LPDU *
LPDU::fromPacket (const CArray & c)
{
  try
  {
    if (c () >= 1)
      {
	if (c[0] == 0xCC)
	  return new L_ACK_PDU (c);
	if (c[0] == 0xC0)
	  return new L_BUSY_PDU (c);
	if (c[0] == 0x0C)
	  return new L_NACK_PDU (c);
	if ((c[0] & 0x53) == 0x10)
	  return new L_Data_PDU (c);
      }
  }
  catch (Exception e)
  {
    return new L_Unknown_PDU (c);
  }
  return new L_Unknown_PDU (c);
}

/* L_NACK */

L_NACK_PDU::L_NACK_PDU ()
{
}

L_NACK_PDU::L_NACK_PDU (const CArray & c)
{
  if (c () != 1)
    throw Exception (PDU_WRONG_FORMAT);
}

CArray L_NACK_PDU::ToPacket ()
{
  uchar
    c = 0x0C;
  return CArray (&c, 1);
}

String L_NACK_PDU::Decode ()
{
  return "NACK";
}

/* L_ACK */

L_ACK_PDU::L_ACK_PDU ()
{
}

L_ACK_PDU::L_ACK_PDU (const CArray & c)
{
  if (c () != 1)
    throw Exception (PDU_WRONG_FORMAT);
}

CArray L_ACK_PDU::ToPacket ()
{
  uchar
    c = 0xCC;
  return CArray (&c, 1);
}

String L_ACK_PDU::Decode ()
{
  return "ACK";
}

/* L_BUSY */

L_BUSY_PDU::L_BUSY_PDU ()
{
}

L_BUSY_PDU::L_BUSY_PDU (const CArray & c)
{
  if (c () != 1)
    throw Exception (PDU_WRONG_FORMAT);
}

CArray L_BUSY_PDU::ToPacket ()
{
  uchar
    c = 0xC0;
  return CArray (&c, 1);
}

String L_BUSY_PDU::Decode ()
{
  return "BUSY";
}

/* L_Unknown  */

L_Unknown_PDU::L_Unknown_PDU ()
{
}

L_Unknown_PDU::L_Unknown_PDU (const CArray & c):pdu (c)
{
}

CArray
L_Unknown_PDU::ToPacket ()
{
  return pdu;
}

String
L_Unknown_PDU::Decode ()
{
  String s ("Unknown LPDU: ");
  unsigned i;

  if (pdu () == 0)
    return "empty LPDU";

  for (i = 0; i < pdu (); i++)
    addHex (s, pdu[i]);

  return s;
}

/* L_Busmonitor  */

L_Busmonitor_PDU::L_Busmonitor_PDU ()
{
}

L_Busmonitor_PDU::L_Busmonitor_PDU (const CArray & c):pdu (c)
{
}

CArray
L_Busmonitor_PDU::ToPacket ()
{
  return pdu;
}

String
L_Busmonitor_PDU::Decode ()
{
  String s ("LPDU: ");
  unsigned i;

  if (pdu () == 0)
    return "empty LPDU";

  for (i = 0; i < pdu (); i++)
    addHex (s, pdu[i]);
  s += ":";
  LPDU *l = LPDU::fromPacket (pdu);
  s += l->Decode ();
  delete l;
  return s;
}

/* L_Data */

L_Data_PDU::L_Data_PDU ()
{
  prio = PRIO_LOW;
  repeated = 0;
  valid_checksum = 1;
  valid_length = 1;
  AddrType = IndividualAddress;
  source = 0;
  dest = 0;
  hopcount = 0x07;
}

L_Data_PDU::L_Data_PDU (const CArray & c)
{
  unsigned len, i;
  uchar c1;
  if (c () < 6)
    throw Exception (PDU_WRONG_FORMAT);
  if ((c[0] & 0x53) != 0x10)
    throw Exception (PDU_WRONG_FORMAT);
  repeated = (c[0] & 0x20) ? 0 : 1;
  valid_length = 1;
  switch ((c[0] >> 2) & 0x3)
    {
    case 0:
      prio = PRIO_SYSTEM;
      break;
    case 1:
      prio = PRIO_URGENT;
      break;
    case 2:
      prio = PRIO_NORMAL;
      break;
    case 3:
      prio = PRIO_LOW;
      break;
    }
  if (c[0] & 0x80)
    {
      /*Standard frame */
      source = (c[1] << 8) | (c[2]);
      dest = (c[3] << 8) | (c[4]);
      len = (c[5] & 0x0f) + 1;
      hopcount = (c[5] >> 4) & 0x07;
      AddrType = (c[5] & 0x80) ? GroupAddress : IndividualAddress;
      if (len + 7 != c ())
	throw Exception (PDU_INCONSISTENT_SIZE);
      data.set (c.array () + 6, len);
      c1 = 0;
      for (i = 0; i < c () - 1; i++)
	c1 ^= c[i];
      c1 = ~c1;
      valid_checksum = (c[c () - 1] == c1 ? 1 : 0);
    }
  else
    {
      /*extended frame */
      if ((c[1] & 0x0f) != 0)
	throw Exception (PDU_WRONG_FORMAT);
      if (c () < 7)
	throw Exception (PDU_WRONG_FORMAT);
      hopcount = (c[1] >> 4) & 0x07;
      AddrType = (c[1] & 0x80) ? GroupAddress : IndividualAddress;
      source = (c[2] << 8) | (c[3]);
      dest = (c[4] << 8) | (c[5]);
      len = c[6] + 1;
      if (len + 8 != c ())
	{
	  if (c () == 23)
	    {
	      valid_length = 0;
	      data.set (c.array () + 7, 8);
	    }
	  else
	    throw Exception (PDU_INCONSISTENT_SIZE);
	}
      else
	data.set (c.array () + 7, len);

      c1 = 0;
      for (i = 0; i < c () - 1; i++)
	c1 ^= c[i];
      c1 = ~c1;
      valid_checksum = (c[c () - 1] == c1 ? 1 : 0);
    }
}

CArray L_Data_PDU::ToPacket ()
{
  assert (data () >= 1);
  assert (data () <= 0xff);
  assert ((hopcount & 0xf8) == 0);
  CArray
    pdu;
  uchar
    c;
  unsigned
    i;
  switch (prio)
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
  if (data () - 1 <= 0x0f)
    {
      pdu.resize (7 + data ());
      pdu[0] = 0x90 | (c << 2) | (repeated ? 0x00 : 0x20);
      pdu[1] = (source >> 8) & 0xff;
      pdu[2] = (source) & 0xff;
      pdu[3] = (dest >> 8) & 0xff;
      pdu[4] = (dest) & 0xff;
      pdu[5] =
	(hopcount & 0x07) << 4 | ((data () - 1) & 0x0f) | (AddrType ==
							   GroupAddress ? 0x80
							   : 0x00);
      pdu.setpart (data.array (), 6, 1 + ((data () - 1) & 0x0f));
    }
  else
    {
      pdu.resize (8 + data ());
      pdu[0] = 0x10 | (c << 2) | (repeated ? 0x00 : 0x20);
      pdu[1] =
	(hopcount & 0x07) << 4 | (AddrType == GroupAddress ? 0x80 : 0x00);
      pdu[2] = (source >> 8) & 0xff;
      pdu[3] = (source) & 0xff;
      pdu[4] = (dest >> 8) & 0xff;
      pdu[5] = (dest) & 0xff;
      pdu[6] = (data () - 1) & 0xff;
      pdu.setpart (data.array (), 7, 1 + ((data () - 1) & 0xff));
    }
  /* checksum */
  c = 0;
  for (i = 0; i < pdu () - 1; i++)
    c ^= pdu[i];
  pdu[pdu () - 1] = ~c;

  return pdu;
}

String L_Data_PDU::Decode ()
{
  assert (data () >= 1);
  assert (data () <= 0xff);
  assert ((hopcount & 0xf8) == 0);

  String
  s ("L_Data");
  if (!valid_length)
    s += " (incomplete)";
  if (repeated)
    s += " (repeated)";
  switch (prio)
    {
    case PRIO_LOW:
      s += " low";
      break;
    case PRIO_NORMAL:
      s += " normal";
      break;
    case PRIO_URGENT:
      s += " urgent";
      break;
    case PRIO_SYSTEM:
      s += " system";
      break;
    }
  if (!valid_checksum)
    s += " INVALID CHECKSUM";
  s =
    s + " from " + FormatEIBAddr (source) + " to " + (AddrType ==
						      GroupAddress ?
						      FormatGroupAddr (dest) :
						      FormatEIBAddr (dest));
  s += " hops: ";
  addHex (s, hopcount);
  TPDU *
    d = TPDU::fromPacket (data);
  s += d->Decode ();
  delete
    d;
  return s;
}
