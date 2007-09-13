/*
    EIBD eib bus access and management daemon
    Copyright (C) 2005-2007 Martin Koegler <mkoegler@auto.tuwien.ac.at>

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

#include "tpdu.h"
#include "apdu.h"

TPDU *
TPDU::fromPacket (const CArray & c)
{
  try
  {
    if (c () >= 1)
      {
	if ((c[0] & 0xfc) == 0)
	  return new T_DATA_XXX_REQ_PDU (c);
	if (c[0] == 0x80)
	  return new T_CONNECT_REQ_PDU (c);
	if (c[0] == 0x81)
	  return new T_DISCONNECT_REQ_PDU (c);
	if ((c[0] & 0xC3) == 0xC2)
	  return new T_ACK_PDU (c);
	if ((c[0] & 0xC3) == 0xC3)
	  return new T_NACK_PDU (c);
	if ((c[0] & 0xC0) == 0x40)
	  return new T_DATA_CONNECTED_REQ_PDU (c);
      }
  }
  catch (Exception e)
  {
    return new T_UNKNOWN_PDU (c);
  }
  return new T_UNKNOWN_PDU (c);
}


/* T_UNKNOWN  */

T_UNKNOWN_PDU::T_UNKNOWN_PDU ()
{
}

T_UNKNOWN_PDU::T_UNKNOWN_PDU (const CArray & c):pdu (c)
{
}

CArray T_UNKNOWN_PDU::ToPacket ()
{
  return pdu;
}

String T_UNKNOWN_PDU::Decode ()
{
  String
  s ("Unknown TPDU: ");
  unsigned
    i;

  if (pdu () == 0)
    return "empty TPDU";

  for (i = 0; i < pdu (); i++)
    addHex (s, pdu[i]);

  return s;
}

/* T_DATA_XXX_REQ  */

T_DATA_XXX_REQ_PDU::T_DATA_XXX_REQ_PDU ()
{
}

T_DATA_XXX_REQ_PDU::T_DATA_XXX_REQ_PDU (const CArray & c):data (c)
{
  if (c () < 1)
    throw
    Exception (PDU_WRONG_FORMAT);
}

CArray T_DATA_XXX_REQ_PDU::ToPacket ()
{
  assert (data () > 0);
  CArray
  pdu (data);
  pdu[0] = (pdu[0] & 0x3);
  return pdu;
}

String T_DATA_XXX_REQ_PDU::Decode ()
{
  APDU *
    a = APDU::fromPacket (data);
  String
  s ("T_DATA_XXX_REQ ");
  s += a->Decode ();
  delete
    a;
  return s;
}

/* T_DATA_CONNECTED_REQ  */

T_DATA_CONNECTED_REQ_PDU::T_DATA_CONNECTED_REQ_PDU ()
{
  serno = 0;
}

T_DATA_CONNECTED_REQ_PDU::T_DATA_CONNECTED_REQ_PDU (const CArray & c):data (c)
{
  if (c () < 1)
    throw
    Exception (PDU_WRONG_FORMAT);
  serno = (c[0] >> 2) & 0x0f;
}

CArray T_DATA_CONNECTED_REQ_PDU::ToPacket ()
{
  assert (data () > 0);
  assert ((serno & 0xf0) == 0);
  CArray
  pdu (data);
  pdu[0] = (pdu[0] & 0x3) | 0x40 | ((serno & 0x0f) << 2);
  return pdu;
}

String T_DATA_CONNECTED_REQ_PDU::Decode ()
{
  assert ((serno & 0xf0) == 0);
  APDU *
    a = APDU::fromPacket (data);
  String
  s ("T_DATA_CONNECTED_REQ serno:");
  addHex (s, serno);
  s += a->Decode ();
  delete
    a;
  return s;
}

/* T_CONNECT_REQ  */

T_CONNECT_REQ_PDU::T_CONNECT_REQ_PDU ()
{
}

T_CONNECT_REQ_PDU::T_CONNECT_REQ_PDU (const CArray & c)
{
  if (c () != 1)
    throw Exception (PDU_WRONG_FORMAT);
}

CArray T_CONNECT_REQ_PDU::ToPacket ()
{
  uchar
    c = 0x80;
  return CArray (&c, 1);
}

String T_CONNECT_REQ_PDU::Decode ()
{
  return "T_CONNECT_REQ";
}

/* T_DISCONNECT_REQ  */

T_DISCONNECT_REQ_PDU::T_DISCONNECT_REQ_PDU ()
{
}

T_DISCONNECT_REQ_PDU::T_DISCONNECT_REQ_PDU (const CArray & c)
{
  if (c () != 1)
    throw Exception (PDU_WRONG_FORMAT);
}

CArray T_DISCONNECT_REQ_PDU::ToPacket ()
{
  uchar
    c = 0x81;
  return CArray (&c, 1);
}

String T_DISCONNECT_REQ_PDU::Decode ()
{
  return "T_DISCONNECT_REQ";
}

/* T_ACK */

T_ACK_PDU::T_ACK_PDU ()
{
  serno = 0;
}

T_ACK_PDU::T_ACK_PDU (const CArray & c)
{
  if (c () != 1)
    throw Exception (PDU_WRONG_FORMAT);
  serno = (c[0] >> 2) & 0x0f;
}

CArray T_ACK_PDU::ToPacket ()
{
  assert ((serno & 0xf0) == 0);
  uchar
    c = 0xC2 | ((serno & 0x0f) << 2);
  return CArray (&c, 1);
}

String T_ACK_PDU::Decode ()
{
  assert ((serno & 0xf0) == 0);
  String
  s ("T_ACK Serno:");
  addHex (s, serno);
  return s;
}

/* T_NACK  */

T_NACK_PDU::T_NACK_PDU ()
{
  serno = 0;
}

T_NACK_PDU::T_NACK_PDU (const CArray & c)
{
  if (c () != 1)
    throw Exception (PDU_WRONG_FORMAT);
  serno = (c[0] >> 2) & 0x0f;
}

CArray T_NACK_PDU::ToPacket ()
{
  assert ((serno & 0xf0) == 0);
  uchar
    c = 0xC3 | ((serno & 0x0f) << 2);
  return CArray (&c, 1);
}

String T_NACK_PDU::Decode ()
{
  assert ((serno & 0xf0) == 0);
  String
  s ("T_NACK Serno:");
  addHex (s, serno);
  return s;
}
