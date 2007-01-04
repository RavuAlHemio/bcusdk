/*
    EIBD eib bus access and management daemon
    Copyright (C) 2005-2007 Martin K�gler <mkoegler@auto.tuwien.ac.at>

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
#include <string.h>
#include "apdu.h"

APDU *
APDU::fromPacket (const CArray & c)
{
  try
  {
    if (c () >= 2)
      {
	switch (c[0] & 0x03)
	  {
	  case 0:
	    switch (c[1] & 0xC0)
	      {
	      case 0x00:
		return new A_GroupValue_Read_PDU (c);
	      case 0x40:
		return new A_GroupValue_Response_PDU (c);
	      case 0x80:
		return new A_GroupValue_Write_PDU (c);
	      case 0xC0:
		return new A_IndividualAddress_Write_PDU (c);
	      }
	    break;
	  case 1:
	    switch (c[1] & 0xC0)
	      {
	      case 0x00:
		return new A_IndividualAddress_Read_PDU (c);
	      case 0x40:
		return new A_IndividualAddress_Response_PDU (c);
	      case 0x80:
		return new A_ADC_Read_PDU (c);
	      case 0xC0:
		return new A_ADC_Response_PDU (c);
	      }
	    break;
	  case 2:
	    switch (c[1] & 0xC0)
	      {
	      case 0x00:
		return new A_Memory_Read_PDU (c);
	      case 0x40:
		return new A_Memory_Response_PDU (c);
	      case 0x80:
		return new A_Memory_Write_PDU (c);
	      case 0xC0:
		switch (c[1])
		  {
		  case 0xC0:
		    return new A_UserMemory_Read_PDU (c);
		  case 0xC1:
		    return new A_UserMemory_Response_PDU (c);
		  case 0xC2:
		    return new A_UserMemory_Write_PDU (c);
		  case 0xC4:
		    return new A_UserMemoryBit_Write_PDU (c);
		  case 0xC5:
		    return new A_UserManufacturerInfo_Read_PDU (c);
		  case 0xC6:
		    return new A_UserManufacturerInfo_Response_PDU (c);
		  }
	      }
	    break;
	  case 3:
	    switch (c[1] & 0xC0)
	      {
	      case 0x00:
		return new A_DeviceDescriptor_Read_PDU (c);
	      case 0x40:
		return new A_DeviceDescriptor_Response_PDU (c);
	      case 0x80:
		return new A_Restart_PDU (c);
	      case 0xC0:
		switch (c[1])
		  {
		  case 0xD0:
		    return new A_MemoryBit_Write_PDU (c);
		  case 0xD1:
		    return new A_Authorize_Request_PDU (c);
		  case 0xD2:
		    return new A_Authorize_Response_PDU (c);
		  case 0xD3:
		    return new A_Key_Write_PDU (c);
		  case 0xD4:
		    return new A_Key_Response_PDU (c);
		  case 0xD5:
		    return new A_PropertyValue_Read_PDU (c);
		  case 0xD6:
		    return new A_PropertyValue_Response_PDU (c);
		  case 0xD7:
		    return new A_PropertyValue_Write_PDU (c);
		  case 0xD8:
		    return new A_PropertyDescription_Read_PDU (c);
		  case 0xD9:
		    return new A_PropertyDescription_Response_PDU (c);
		  case 0xDC:
		    return new A_IndividualAddressSerialNumber_Read_PDU (c);
		  case 0xDD:
		    return new
		      A_IndividualAddressSerialNumber_Response_PDU (c);
		  case 0xDE:
		    return new A_IndividualAddressSerialNumber_Write_PDU (c);
		  case 0xDF:
		    return new A_ServiceInformation_Indication_Write_PDU (c);
		  case 0xE0:
		    return new A_DomainAddress_Write_PDU (c);
		  case 0xE1:
		    return new A_DomainAddress_Read_PDU (c);
		  case 0xE2:
		    return new A_DomainAddress_Response_PDU (c);
		  case 0xE3:
		    return new A_DomainAddressSelective_Read_PDU (c);
		  }
	      }
	    break;
	  }
      }
  }
  catch (Exception e)
  {
    return new A_Unknown_PDU (c);
  }
  return new A_Unknown_PDU (c);
}

/* A_Unknown_PDU */

A_Unknown_PDU::A_Unknown_PDU ()
{
}

A_Unknown_PDU::A_Unknown_PDU (const CArray & c):pdu (c)
{
}

CArray A_Unknown_PDU::ToPacket ()
{
  return pdu;
}

String A_Unknown_PDU::Decode ()
{
  String
  s ("Unknown APDU: ");
  unsigned 
    i;

  if (pdu () == 0)
    return "empty APDU";
  addHex (s, pdu[0] & 0x03);

  for (i = 1; i < pdu (); i++)
    addHex (s, pdu[i]);

  return s;
}

bool A_Unknown_PDU::isResponse(const APDU* req) const
{
  return 0;
}

/* A_GroupValue_Read */

A_GroupValue_Read_PDU::A_GroupValue_Read_PDU ()
{
}

A_GroupValue_Read_PDU::A_GroupValue_Read_PDU (const CArray & c)
{
  if (c () != 2)
    throw Exception (PDU_WRONG_FORMAT);
}

CArray A_GroupValue_Read_PDU::ToPacket ()
{
  uchar
    c[2] = {
    0x00, 0x00
  };
  return CArray (c, 2);
}

String A_GroupValue_Read_PDU::Decode ()
{
  return "A_GroupValue_Read";
}

bool A_GroupValue_Read_PDU::isResponse(const APDU* req) const
{
  return 0;
}

/* A_GroupValue_Response */

A_GroupValue_Response_PDU::A_GroupValue_Response_PDU ()
{
  issmall = 0;
}

A_GroupValue_Response_PDU::A_GroupValue_Response_PDU (const CArray & c)
{
  if (c () < 2)
    throw Exception (PDU_WRONG_FORMAT);
  if (c () == 2)
    {
      uchar c1 = c[1] & 0x3f;
      data.set (&c1, 1);
      issmall = 1;
    }
  else
    {
      data.set (c.array () + 2, c () - 2);
      issmall = 0;
    }
}

CArray A_GroupValue_Response_PDU::ToPacket ()
{
  CArray
    pdu;
  assert (!issmall || (data () == 1 && (data[0] & 0xC0) == 0));
  if (issmall)
    {
      pdu.resize (2);
      pdu[0] = 0x00;
      pdu[1] = 0x40 | (data[0] & 0x3f);
      return pdu;
    }
  pdu.resize (2 + data ());
  pdu[0] = 0x00;
  pdu[1] = 0x40;
  pdu.setpart (data.array (), 2, data ());
  return pdu;
}

String A_GroupValue_Response_PDU::Decode ()
{
  unsigned
    i;
  assert (!issmall || (data () == 1 && (data[0] & 0xC0) == 0));
  String
  s ("A_GroupValue_Response ");
  if (issmall)
    s += "(small) ";

  for (i = 0; i < data (); i++)
    addHex (s, data[i]);

  return s;
}

bool A_GroupValue_Response_PDU::isResponse(const APDU* req) const
{
  return req->getType()==A_GroupValue_Read;
}

/* A_GroupValue_Write */

A_GroupValue_Write_PDU::A_GroupValue_Write_PDU ()
{
  issmall = 0;
}

A_GroupValue_Write_PDU::A_GroupValue_Write_PDU (const CArray & c)
{
  if (c () < 2)
    throw Exception (PDU_WRONG_FORMAT);
  if (c () == 2)
    {
      uchar c1 = c[1] & 0x3f;
      data.set (&c1, 1);
      issmall = 1;
    }
  else
    {
      data.set (c.array () + 2, c () - 2);
      issmall = 0;
    }
}

CArray A_GroupValue_Write_PDU::ToPacket ()
{
  CArray
    pdu;
  assert (!issmall || (data () == 1 && (data[0] & 0xC0) == 0));
  if (issmall)
    {
      pdu.resize (2);
      pdu[0] = 0x00;
      pdu[1] = 0x80 | (data[0] & 0x3F);
      return pdu;
    }
  pdu.resize (2 + data ());
  pdu[0] = 0x00;
  pdu[1] = 0x80;
  pdu.setpart (data.array (), 2, data ());
  return pdu;
}

String A_GroupValue_Write_PDU::Decode ()
{
  unsigned
    i;
  assert (!issmall || (data () == 1 && (data[0] & 0xC0) == 0));
  String
  s ("A_GroupValue_Write ");
  if (issmall)
    s += "(small) ";

  for (i = 0; i < data (); i++)
    addHex (s, data[i]);

  return s;
}

bool A_GroupValue_Write_PDU::isResponse(const APDU* req) const
{
  return 0;
}

/* A_IndividualAddress_Write */

A_IndividualAddress_Write_PDU::A_IndividualAddress_Write_PDU ()
{
  addr = 0;
}

A_IndividualAddress_Write_PDU::
A_IndividualAddress_Write_PDU (const CArray & c)
{
  if (c () != 4)
    throw Exception (PDU_WRONG_FORMAT);
  addr = (c[2] << 8) | (c[3]);
}

CArray A_IndividualAddress_Write_PDU::ToPacket ()
{
  CArray
    pdu;
  pdu.resize (4);
  pdu[0] = 0;
  pdu[1] = 0xC0;
  pdu[2] = (addr >> 8) & 0xff;
  pdu[3] = addr & 0xff;
  return pdu;
}

String A_IndividualAddress_Write_PDU::Decode ()
{
  String
  s ("A_IndividualAddress_Write ");
  return s + FormatEIBAddr (addr);
}

bool A_IndividualAddress_Write_PDU::isResponse(const APDU* req) const
{
  return 0;
}

/* A_IndividualAddress_Read */

A_IndividualAddress_Read_PDU::A_IndividualAddress_Read_PDU ()
{
}

A_IndividualAddress_Read_PDU::A_IndividualAddress_Read_PDU (const CArray & c)
{
  if (c () != 2)
    throw Exception (PDU_WRONG_FORMAT);
}

CArray A_IndividualAddress_Read_PDU::ToPacket ()
{
  uchar
    c[2] = {
    0x01, 0x00
  };
  return CArray (c, 2);
}

String A_IndividualAddress_Read_PDU::Decode ()
{
  return "A_IndividualAddress_Read";
}

bool A_IndividualAddress_Read_PDU::isResponse(const APDU* req) const
{
  return 0;
}

/* A_IndividualAddress_Response */

A_IndividualAddress_Response_PDU::A_IndividualAddress_Response_PDU ()
{
}

A_IndividualAddress_Response_PDU::
A_IndividualAddress_Response_PDU (const CArray & c)
{
  if (c () != 2)
    throw Exception (PDU_WRONG_FORMAT);
}

CArray A_IndividualAddress_Response_PDU::ToPacket ()
{
  uchar
    c[2] = {
    0x01, 0x40
  };
  return CArray (c, 2);
}

String A_IndividualAddress_Response_PDU::Decode ()
{
  return "A_IndividualAddress_Response";
}

bool A_IndividualAddress_Response_PDU::isResponse(const APDU* req) const
{
  return req->getType()==A_IndividualAddress_Read;
}

/* A_IndividualAddressSerialNumber_Read */

A_IndividualAddressSerialNumber_Read_PDU::
A_IndividualAddressSerialNumber_Read_PDU ()
{
  memset (serno, 0, sizeof (serno));
}

A_IndividualAddressSerialNumber_Read_PDU::
A_IndividualAddressSerialNumber_Read_PDU (const CArray & c)
{
  if (c () != 8)
    throw Exception (PDU_WRONG_FORMAT);
  memcpy (serno, c.array () + 2, 6);
}

CArray A_IndividualAddressSerialNumber_Read_PDU::ToPacket ()
{
  CArray
    pdu;
  pdu.resize (8);
  pdu[0] = 0x03;
  pdu[1] = 0xDC;
  pdu.setpart (serno, 2, 6);
  return pdu;
}

String A_IndividualAddressSerialNumber_Read_PDU::Decode ()
{
  String
  s ("A_IndividualAddressSerialNumber_Read ");
  addHex (s, serno[0]);
  addHex (s, serno[1]);
  addHex (s, serno[2]);
  addHex (s, serno[3]);
  addHex (s, serno[4]);
  addHex (s, serno[5]);
  return s;
}

bool A_IndividualAddressSerialNumber_Read_PDU::isResponse(const APDU* req) const
{
  return 0;
}

/* A_IndividualAddressSerialNumber_Response */

A_IndividualAddressSerialNumber_Response_PDU::
A_IndividualAddressSerialNumber_Response_PDU ()
{
  memset (serno, 0, sizeof (serno));
  addr = 0;
}

A_IndividualAddressSerialNumber_Response_PDU::
A_IndividualAddressSerialNumber_Response_PDU (const CArray & c)
{
  if (c () != 12)
    throw Exception (PDU_WRONG_FORMAT);
  memcpy (serno, c.array () + 2, 6);
  addr = (c[8] << 8) | (c[9]);
}

CArray A_IndividualAddressSerialNumber_Response_PDU::ToPacket ()
{
  CArray
    pdu;
  pdu.resize (12);
  pdu[0] = 0x03;
  pdu[1] = 0xDD;
  pdu.setpart (serno, 2, 6);
  pdu[8] = (addr >> 8) & 0xff;
  pdu[9] = (addr) & 0xff;
  pdu[10] = 0;
  pdu[11] = 0;
  return pdu;
}

String A_IndividualAddressSerialNumber_Response_PDU::Decode ()
{
  String
  s ("A_IndividualAddressSerialNumber_Response ");
  addHex (s, serno[0]);
  addHex (s, serno[1]);
  addHex (s, serno[2]);
  addHex (s, serno[3]);
  addHex (s, serno[4]);
  addHex (s, serno[5]);
  s += "Addr: ";
  add16Hex (s, addr);
  return s;
}

bool A_IndividualAddressSerialNumber_Response_PDU::isResponse(const APDU* req) const
{
  if(req->getType()!=A_IndividualAddressSerialNumber_Read)
    return 0;
  const A_IndividualAddressSerialNumber_Read_PDU* a=
    (const A_IndividualAddressSerialNumber_Read_PDU*)req;
  if(memcmp(a->serno,serno,sizeof(serno)))
    return 0;
  return 1;
}

/* A_IndividualAddressSerialNumber_Write */

A_IndividualAddressSerialNumber_Write_PDU::
A_IndividualAddressSerialNumber_Write_PDU ()
{
  memset (serno, 0, sizeof (serno));
  addr = 0;
}

A_IndividualAddressSerialNumber_Write_PDU::
A_IndividualAddressSerialNumber_Write_PDU (const CArray & c)
{
  if (c () != 14)
    throw Exception (PDU_WRONG_FORMAT);
  memcpy (serno, c.array () + 2, 6);
  addr = (c[8] << 8) | (c[9]);
}

CArray A_IndividualAddressSerialNumber_Write_PDU::ToPacket ()
{
  CArray
    pdu;
  pdu.resize (14);
  pdu[0] = 0x03;
  pdu[1] = 0xDD;
  pdu.setpart (serno, 2, 6);
  pdu[8] = (addr >> 8) & 0xff;
  pdu[9] = (addr) & 0xff;
  pdu[10] = 0;
  pdu[11] = 0;
  pdu[12] = 0;
  pdu[13] = 0;
  return pdu;
}

String A_IndividualAddressSerialNumber_Write_PDU::Decode ()
{
  String
  s ("A_IndividualAddressSerialNumber_Write ");
  addHex (s, serno[0]);
  addHex (s, serno[1]);
  addHex (s, serno[2]);
  addHex (s, serno[3]);
  addHex (s, serno[4]);
  addHex (s, serno[5]);
  s += "Addr: ";
  s += FormatEIBAddr (addr);
  return s;
}

bool A_IndividualAddressSerialNumber_Write_PDU::isResponse(const APDU* req) const
{
  return 0;
}

/* A_ServiceInformation_Indication_Write_PDU */

A_ServiceInformation_Indication_Write_PDU::
A_ServiceInformation_Indication_Write_PDU ()
{
  verify_mode = 0;
  duplicate_address = 0;
  appl_stopped = 0;
}

A_ServiceInformation_Indication_Write_PDU::
A_ServiceInformation_Indication_Write_PDU (const CArray & c)
{
  if (c () != 5)
    throw Exception (PDU_WRONG_FORMAT);
  verify_mode = (c[2] & 0x04) ? 1 : 0;
  duplicate_address = (c[3] & 0x02) ? 1 : 0;
  appl_stopped = (c[2] & 0x01) ? 1 : 0;
}

CArray
A_ServiceInformation_Indication_Write_PDU::ToPacket ()
{
  CArray pdu;
  pdu.resize (5);
  pdu[0] = 0x03;
  pdu[1] = 0xDF;
  pdu[2] =
    0x00 | (verify_mode ? 0x04 : 0x00) | (duplicate_address ? 0x02 : 0x00) |
    (appl_stopped ? 0x01 : 0x00);
  pdu[3] = 0x00;
  pdu[4] = 0x00;
  return pdu;
}

String
A_ServiceInformation_Indication_Write_PDU::Decode ()
{
  String s ("A_ServiceInformation_Indication_Write ");
  if (verify_mode)
    s += "verify ";
  if (duplicate_address)
    s += "dupplicate_address ";
  if (appl_stopped)
    s += "appl_stopped ";

  return s;
}

bool A_ServiceInformation_Indication_Write_PDU::isResponse(const APDU* req) const
{
  return 0;
}

/* A_DomainAddress_Write */

A_DomainAddress_Write_PDU::A_DomainAddress_Write_PDU ()
{
  addr = 0;
}

A_DomainAddress_Write_PDU::A_DomainAddress_Write_PDU (const CArray & c)
{
  if (c () != 4)
    throw Exception (PDU_WRONG_FORMAT);
  addr = (c[2] << 8) | (c[3]);
}

CArray A_DomainAddress_Write_PDU::ToPacket ()
{
  CArray
    pdu;
  pdu.resize (4);
  pdu[0] = 0x03;
  pdu[1] = 0xE0;
  pdu[2] = (addr >> 8) & 0xff;
  pdu[3] = addr & 0xff;
  return pdu;
}

String A_DomainAddress_Write_PDU::Decode ()
{
  String
  s ("A_DomainAddress_Write ");
  return s + FormatDomainAddr (addr);
}


bool A_DomainAddress_Write_PDU::isResponse(const APDU* req) const
{
  return 0;
}

/* A_DomainAddress_Read */

A_DomainAddress_Read_PDU::A_DomainAddress_Read_PDU ()
{
}

A_DomainAddress_Read_PDU::A_DomainAddress_Read_PDU (const CArray & c)
{
  if (c () != 2)
    throw Exception (PDU_WRONG_FORMAT);
}

CArray A_DomainAddress_Read_PDU::ToPacket ()
{
  uchar
    c[2] = {
    0x03, 0xE1
  };
  return CArray (c, 2);
}

String A_DomainAddress_Read_PDU::Decode ()
{
  return "A_DomainAddress_Read";
}

bool A_DomainAddress_Read_PDU::isResponse(const APDU* req) const
{
  return 0;
}

/* A_DomainAddress_Response */

A_DomainAddress_Response_PDU::A_DomainAddress_Response_PDU ()
{
  addr = 0;
}

A_DomainAddress_Response_PDU::A_DomainAddress_Response_PDU (const CArray & c)
{
  if (c () != 4)
    throw Exception (PDU_WRONG_FORMAT);
  addr = (c[2] << 8) | c[3];
}

CArray A_DomainAddress_Response_PDU::ToPacket ()
{
  CArray
    pdu;
  pdu.resize (4);
  pdu[0] = 0x03;
  pdu[1] = 0xE2;
  pdu[2] = (addr >> 8) & 0xff;
  pdu[3] = (addr) & 0xff;
  return pdu;
}

String A_DomainAddress_Response_PDU::Decode ()
{
  String
  s ("A_DomainAddress_Response");
  s += FormatDomainAddr (addr);
  return s;
}

bool A_DomainAddress_Response_PDU::isResponse(const APDU* req) const
{
  return req->getType()==A_DomainAddress_Read;
}

/* A_DomainAddressSelective_Read */

A_DomainAddressSelective_Read_PDU::A_DomainAddressSelective_Read_PDU ()
{
  domainaddr = 0;
  addr = 0;
  range = 0;
}

A_DomainAddressSelective_Read_PDU::
A_DomainAddressSelective_Read_PDU (const CArray & c)
{
  if (c () != 7)
    throw Exception (PDU_WRONG_FORMAT);
  domainaddr = (c[2] << 8) | c[3];
  addr = (c[4] << 8) | c[5];
  range = c[6];
}

CArray A_DomainAddressSelective_Read_PDU::ToPacket ()
{
  CArray
    pdu;
  pdu.resize (7);
  pdu[0] = 0x03;
  pdu[1] = 0xE2;
  pdu[2] = (domainaddr >> 8) & 0xff;
  pdu[3] = (domainaddr) & 0xff;
  pdu[2] = (addr >> 8) & 0xff;
  pdu[3] = (addr) & 0xff;
  pdu[6] = range;
  return pdu;
}

String
A_DomainAddressSelective_Read_PDU::Decode ()
{
  String s ("A_DomainAddressSelective_Read ");
  s += FormatDomainAddr (domainaddr);
  s += " ";
  s += FormatEIBAddr (addr);
  s += " ";
  addHex (s, range);
  return s;
}

bool A_DomainAddressSelective_Read_PDU::isResponse(const APDU* req) const
{
  return 0;
}

/* A_PropertyValue_Read */

A_PropertyValue_Read_PDU::A_PropertyValue_Read_PDU ()
{
  obj = 0;
  prop = 0;
  count = 0;
  start = 0;
}

A_PropertyValue_Read_PDU::A_PropertyValue_Read_PDU (const CArray & c)
{
  if (c () != 6)
    throw Exception (PDU_WRONG_FORMAT);
  obj = c[2];
  prop = c[3];
  count = (c[4] >> 4) & 0x0f;
  start = (c[4] & 0x0f) << 8 | c[5];
}

CArray
A_PropertyValue_Read_PDU::ToPacket ()
{
  CArray pdu;
  assert ((count & 0xf0) == 0);
  assert ((start & 0xf000) == 0);
  pdu.resize (6);
  pdu[0] = 0x03;
  pdu[1] = 0xD5;
  pdu[2] = obj;
  pdu[3] = prop;
  pdu[4] = (count & 0x0f) << 4 | (start >> 8) & 0x0f;
  pdu[5] = start & 0xff;
  return pdu;
}

String
A_PropertyValue_Read_PDU::Decode ()
{
  assert ((count & 0xf0) == 0);
  assert ((start & 0xf000) == 0);
  String s ("A_PropertyValue_Read Obj:");
  addHex (s, obj);
  s += " Prop: ";
  addHex (s, prop);
  s += " start: ";
  addHex (s, start);
  s += " max_nr: ";
  addHex (s, count);
  return s;
}

bool A_PropertyValue_Read_PDU::isResponse(const APDU* req) const
{
  return 0;
}

/* A_PropertyValue_Response */

A_PropertyValue_Response_PDU::A_PropertyValue_Response_PDU ()
{
  obj = 0;
  prop = 0;
  count = 0;
  start = 0;
}

A_PropertyValue_Response_PDU::A_PropertyValue_Response_PDU (const CArray & c)
{
  if (c () < 6)
    throw Exception (PDU_WRONG_FORMAT);
  obj = c[2];
  prop = c[3];
  count = (c[4] >> 4) & 0x0f;
  start = (c[4] & 0x0f) << 8 | c[5];
  data.set (c.array () + 6, c () - 6);
}

CArray
A_PropertyValue_Response_PDU::ToPacket ()
{
  CArray pdu;
  assert ((count & 0xf0) == 0);
  assert ((start & 0xf000) == 0);
  pdu.resize (6 + data ());
  pdu[0] = 0x03;
  pdu[1] = 0xD6;
  pdu[2] = obj;
  pdu[3] = prop;
  pdu[4] = (count & 0x0f) << 4 | (start >> 8) & 0x0f;
  pdu[5] = start & 0xff;
  pdu.setpart (data.array (), 6, data ());
  return pdu;
}

String
A_PropertyValue_Response_PDU::Decode ()
{
  assert ((count & 0xf0) == 0);
  assert ((start & 0xf000) == 0);
  String s ("A_PropertyValue_Response Obj:");
  addHex (s, obj);
  s += " Prop: ";
  addHex (s, prop);
  s += " start: ";
  addHex (s, start);
  s += " max_nr: ";
  addHex (s, count);
  s += "data: ";
  for (unsigned i = 0; i < data (); i++)
    addHex (s, data[i]);
  return s;
}

bool A_PropertyValue_Response_PDU::isResponse(const APDU* req) const
{
  if(req->getType()==A_PropertyValue_Write)
    {
      const A_PropertyValue_Write_PDU* a=(const A_PropertyValue_Write_PDU*)req;
      if(a->obj==obj&&a->prop==prop&&a->start==start&&a->count==count)
	return 1;
    }

  if(req->getType()!=A_PropertyValue_Read)
   return 0;
  const A_PropertyValue_Read_PDU* a=(const A_PropertyValue_Read_PDU*)req;
  if(a->obj!=obj)
    return 0;
  if(a->prop!=prop)
    return 0;
  if(a->start!=start)
    return 0;
  if(a->count!=count)
    return 0;
  return 1;
}

/* A_PropertyValue_Write */

A_PropertyValue_Write_PDU::A_PropertyValue_Write_PDU ()
{
  obj = 0;
  prop = 0;
  count = 0;
  start = 0;
}

A_PropertyValue_Write_PDU::A_PropertyValue_Write_PDU (const CArray & c)
{
  if (c () < 6)
    throw Exception (PDU_WRONG_FORMAT);
  obj = c[2];
  prop = c[3];
  count = (c[4] >> 4) & 0x0f;
  start = (c[4] & 0x0f) << 8 | c[5];
  data.set (c.array () + 6, c () - 6);
}

CArray
A_PropertyValue_Write_PDU::ToPacket ()
{
  CArray pdu;
  assert ((count & 0xf0) == 0);
  assert ((start & 0xf000) == 0);
  pdu.resize (6 + data ());
  pdu[0] = 0x03;
  pdu[1] = 0xD7;
  pdu[2] = obj;
  pdu[3] = prop;
  pdu[4] = (count & 0x0f) << 4 | (start >> 8) & 0x0f;
  pdu[5] = start & 0xff;
  pdu.setpart (data.array (), 6, data ());
  return pdu;
}

String
A_PropertyValue_Write_PDU::Decode ()
{
  assert ((count & 0xf0) == 0);
  assert ((start & 0xf000) == 0);
  String s ("A_PropertyValue_Write Obj:");
  addHex (s, obj);
  s += " Prop: ";
  addHex (s, prop);
  s += " start: ";
  addHex (s, start);
  s += " max_nr: ";
  addHex (s, count);
  s += "data: ";
  for (unsigned i = 0; i < data (); i++)
    addHex (s, data[i]);
  return s;
}

bool A_PropertyValue_Write_PDU::isResponse(const APDU* req) const
{
  return 0;
}

/* A_PropertyDescription_Read */

A_PropertyDescription_Read_PDU::A_PropertyDescription_Read_PDU ()
{
  obj = 0;
  prop = 0;
  property_index = 0;
}

A_PropertyDescription_Read_PDU::
A_PropertyDescription_Read_PDU (const CArray & c)
{
  if (c () != 5)
    throw Exception (PDU_WRONG_FORMAT);
  obj = c[2];
  prop = c[3];
  property_index = c[4];
}

CArray
A_PropertyDescription_Read_PDU::ToPacket ()
{
  CArray pdu;
  pdu.resize (5);
  pdu[0] = 0x03;
  pdu[1] = 0xD8;
  pdu[2] = obj;
  pdu[3] = prop;
  pdu[4] = property_index;
  return pdu;
}

String
A_PropertyDescription_Read_PDU::Decode ()
{
  String s ("A_PropertyDescription_Read Obj: ");
  addHex (s, obj);
  s += " Property: ";
  addHex (s, prop);
  s += " Property_index: ";
  addHex (s, property_index);
  return s;
}

bool A_PropertyDescription_Read_PDU::isResponse(const APDU* req) const
{
  return 0;
}

/* A_PropertyDescription_Response */

A_PropertyDescription_Response_PDU::A_PropertyDescription_Response_PDU ()
{
  obj = 0;
  prop = 0;
  property_index = 0;
  type = 0;
  count = 0;
  access = 0;
}

A_PropertyDescription_Response_PDU::
A_PropertyDescription_Response_PDU (const CArray & c)
{
  if (c () != 9)
    throw Exception (PDU_WRONG_FORMAT);
  obj = c[2];
  prop = c[3];
  property_index = c[4];
  type = c[5];
  count = (c[6] << 8) | c[7];
  access = c[8];
}

CArray
A_PropertyDescription_Response_PDU::ToPacket ()
{
  CArray pdu;
  pdu.resize (9);
  pdu[0] = 0x03;
  pdu[1] = 0xD9;
  pdu[2] = obj;
  pdu[3] = prop;
  pdu[4] = property_index;
  pdu[5] = type;
  pdu[6] = (count >> 8) & 0xff;
  pdu[7] = (count) & 0xff;
  pdu[8] = access;
  return pdu;
}

String
A_PropertyDescription_Response_PDU::Decode ()
{
  String s ("A_PropertyDescription_Response Obj:");
  addHex (s, obj);
  s += " Property: ";
  addHex (s, prop);
  s += " Property_index: ";
  addHex (s, property_index);
  s += " Type: ";
  addHex (s, type);
  s += "max_elements: ";
  add16Hex (s, count);
  s += " acces: ";
  addHex (s, access);
  return s;
}

bool A_PropertyDescription_Response_PDU::isResponse(const APDU* req) const
{
  if(req->getType()!=A_PropertyDescription_Read)
   return 0;
  const A_PropertyDescription_Read_PDU* a=(const A_PropertyDescription_Read_PDU*)req;
  if(a->obj!=obj)
    return 0;
  return 1;
}

/* A_DeviceDescriptor_Read */

A_DeviceDescriptor_Read_PDU::A_DeviceDescriptor_Read_PDU ()
{
  type = 0;
}

A_DeviceDescriptor_Read_PDU::A_DeviceDescriptor_Read_PDU (const CArray & c)
{
  if (c () != 2)
    throw Exception (PDU_WRONG_FORMAT);
  type = c[1] & 0x3F;
}

CArray
A_DeviceDescriptor_Read_PDU::ToPacket ()
{
  assert ((type & 0xC0) == 0);
  CArray pdu;
  pdu.resize (2);
  pdu[0] = 0x03;
  pdu[1] = 0x00 | (type & 0x3f);
  return pdu;
}

String
A_DeviceDescriptor_Read_PDU::Decode ()
{
  assert ((type & 0xC0) == 0);
  String s ("A_DeviceDescriptor_Read Type:");
  addHex (s, type);
  return s;
}

bool A_DeviceDescriptor_Read_PDU::isResponse(const APDU* req) const
{
  return 0;
}

/* A_DeviceDescriptor_Response */

A_DeviceDescriptor_Response_PDU::A_DeviceDescriptor_Response_PDU ()
{
  type = 0;
  descriptor = 0;
}

A_DeviceDescriptor_Response_PDU::
A_DeviceDescriptor_Response_PDU (const CArray & c)
{
  if (c () != 4)
    throw Exception (PDU_WRONG_FORMAT);
  type = c[1] & 0x3F;
  descriptor = (c[2] << 8) | c[3];
}

CArray
A_DeviceDescriptor_Response_PDU::ToPacket ()
{
  assert ((type & 0xC0) == 0);
  CArray pdu;
  pdu.resize (4);
  pdu[0] = 0x03;
  pdu[1] = 0x01 | (type & 0x3f);
  pdu[2] = (descriptor >> 8) & 0xFF;
  pdu[3] = (descriptor) & 0xff;
  return pdu;
}

String
A_DeviceDescriptor_Response_PDU::Decode ()
{
  assert ((type & 0xC0) == 0);
  String s ("A_DeviceDescriptor_Response Type:");
  addHex (s, type);
  s += " Descriptor: ";
  add16Hex (s, descriptor);
  return s;
}

bool A_DeviceDescriptor_Response_PDU::isResponse(const APDU* req) const
{
  if(req->getType()!=A_DeviceDescriptor_Read)
   return 0;
  const A_DeviceDescriptor_Read_PDU* a=(const A_DeviceDescriptor_Read_PDU*)req;
  if(a->type!=type)
    return 0;
  return 1;
}

/* A_ADC_Read */

A_ADC_Read_PDU::A_ADC_Read_PDU ()
{
  channel = 0;
  count = 0;
}

A_ADC_Read_PDU::A_ADC_Read_PDU (const CArray & c)
{
  if (c () != 3)
    throw Exception (PDU_WRONG_FORMAT);
  channel = c[1] & 0x3F;
  count = c[2];
}

CArray
A_ADC_Read_PDU::ToPacket ()
{
  assert ((channel & 0xC0) == 0);
  CArray pdu;
  pdu.resize (3);
  pdu[0] = 0x01;
  pdu[1] = 0x80 | (channel & 0x3F);
  pdu[2] = count;
  return pdu;
}

String
A_ADC_Read_PDU::Decode ()
{
  assert ((channel & 0xC0) == 0);
  String s ("A_ADC_Read Channel:");
  addHex (s, channel);
  s += " Count: ";
  addHex (s, count);
  return s;
}

bool A_ADC_Read_PDU::isResponse(const APDU* req) const
{
  return 0;
}

/* A_ADC_Response */

A_ADC_Response_PDU::A_ADC_Response_PDU ()
{
  channel = 0;
  count = 0;
  val = 0;
}

A_ADC_Response_PDU::A_ADC_Response_PDU (const CArray & c)
{
  if (c () != 5)
    throw Exception (PDU_WRONG_FORMAT);
  channel = c[1] & 0x3F;
  count = c[2];
  val = (c[3] << 8) | (c[4]);
}

CArray
A_ADC_Response_PDU::ToPacket ()
{
  assert ((channel & 0xC0) == 0);
  CArray pdu;
  pdu.resize (5);
  pdu[0] = 0x01;
  pdu[1] = 0xC0 | (channel & 0x3F);
  pdu[2] = count;
  pdu[3] = (val >> 8) & 0xff;
  pdu[4] = (val) & 0xff;
  return pdu;
}

String
A_ADC_Response_PDU::Decode ()
{
  assert ((channel & 0xC0) == 0);
  String s ("A_ADC_Response Channel:");
  addHex (s, channel);
  s += " Count: ";
  addHex (s, count);
  s += "Value: ";
  addHex (s, val);
  return s;
}

bool A_ADC_Response_PDU::isResponse(const APDU* req) const
{
  if(req->getType()!=A_ADC_Read)
    return 0;
  const A_ADC_Read_PDU* a=(const A_ADC_Read_PDU*)req;
  if(a->count!=count)
    return 0;
  if(a->channel!=channel)
    return 0;
  return 1;
}

/* A_Memory_Read */

A_Memory_Read_PDU::A_Memory_Read_PDU ()
{
  count = 0;
  addr = 0;
}

A_Memory_Read_PDU::A_Memory_Read_PDU (const CArray & c)
{
  if (c () != 4)
    throw Exception (PDU_WRONG_FORMAT);
  count = c[1] & 0xf;
  addr = (c[2] << 8) | c[3];
}

CArray
A_Memory_Read_PDU::ToPacket ()
{
  assert ((count & 0xf0) == 0);
  CArray pdu;
  pdu.resize (4);
  pdu[0] = 0x02;
  pdu[1] = 0x00 | (count & 0x0f);
  pdu[2] = (addr >> 8) & 0xff;
  pdu[3] = addr & 0xff;
  return pdu;
}

String
A_Memory_Read_PDU::Decode ()
{
  assert ((count & 0xf0) == 0);
  String s ("A_Memory_Read Len: ");
  addHex (s, count);
  s += " Addr: ";
  add16Hex (s, addr);
  return s;
}

bool A_Memory_Read_PDU::isResponse(const APDU* req) const
{
  return 0;
}

/* A_Memory_Response */

A_Memory_Response_PDU::A_Memory_Response_PDU ()
{
  count = 0;
  addr = 0;
}

A_Memory_Response_PDU::A_Memory_Response_PDU (const CArray & c)
{
  if (c () < 4)
    throw Exception (PDU_WRONG_FORMAT);
  count = c[1] & 0xf;
  addr = (c[2] << 8) | c[3];
  data.set (c.array () + 4, c () - 4);
  if (data () != count)
    throw Exception (PDU_INCONSISTENT_SIZE);
}

CArray
A_Memory_Response_PDU::ToPacket ()
{
  assert ((count & 0xf0) == 0);
  assert (data () == count);
  CArray pdu;
  pdu.resize (4 + data ());
  pdu[0] = 0x02;
  pdu[1] = 0x40 | (count & 0x0f);
  pdu[2] = (addr >> 8) & 0xff;
  pdu[3] = addr & 0xff;
  pdu.setpart (data.array (), 4, data ());
  return pdu;
}

String
A_Memory_Response_PDU::Decode ()
{
  assert ((count & 0xf0) == 0);
  assert (data () == count);
  String s ("A_Memory_Response Len:");
  addHex (s, count);
  s += " Addr: ";
  add16Hex (s, addr);
  s += "Data: ";
  for (unsigned i = 0; i < data (); i++)
    addHex (s, data[i]);
  return s;
}

bool A_Memory_Response_PDU::isResponse(const APDU* req) const
{
  if(req->getType()!=A_Memory_Read)
    return 0;
  const A_Memory_Read_PDU* a=(const A_Memory_Read_PDU*)req;
  if(a->count!=count)
    return 0;
  if(a->addr!=addr)
    return 0;
  return 1;
}

/* A_Memory_Write */

A_Memory_Write_PDU::A_Memory_Write_PDU ()
{
  count = 0;
  addr = 0;
}

A_Memory_Write_PDU::A_Memory_Write_PDU (const CArray & c)
{
  if (c () < 4)
    throw Exception (PDU_WRONG_FORMAT);
  count = c[1] & 0xf;
  addr = (c[2] << 8) | c[3];
  data.set (c.array () + 4, c () - 4);
  if (data () != count)
    throw Exception (PDU_INCONSISTENT_SIZE);
}

CArray
A_Memory_Write_PDU::ToPacket ()
{
  assert ((count & 0xf0) == 0);
  assert (data () == count);
  CArray pdu;
  pdu.resize (4 + data ());
  pdu[0] = 0x02;
  pdu[1] = 0x80 | (count & 0x0f);
  pdu[2] = (addr >> 8) & 0xff;
  pdu[3] = addr & 0xff;
  pdu.setpart (data.array (), 4, data ());
  return pdu;
}

String
A_Memory_Write_PDU::Decode ()
{
  assert ((count & 0xf0) == 0);
  assert (data () == count);
  String s ("A_Memory_Write Len:");
  addHex (s, count);
  s += " Addr: ";
  add16Hex (s, addr);
  s += "Data: ";
  for (unsigned i = 0; i < data (); i++)
    addHex (s, data[i]);
  return s;
}

bool A_Memory_Write_PDU::isResponse(const APDU* req) const
{
  return 0;
}

/* A_MemoryBit_Write */

A_MemoryBit_Write_PDU::A_MemoryBit_Write_PDU ()
{
  count = 0;
  addr = 0;
}

A_MemoryBit_Write_PDU::A_MemoryBit_Write_PDU (const CArray & c)
{
  if (c () < 5)
    throw Exception (PDU_WRONG_FORMAT);
  count = c[2];
  addr = (c[3] << 8) | c[4];
  if (c () - 5 != count * 2)
    throw Exception (PDU_INCONSISTENT_SIZE);
  andmask.set (c.array () + 5, count);
  xormask.set (c.array () + 5 + count, count);
}

CArray
A_MemoryBit_Write_PDU::ToPacket ()
{
  assert (andmask () == count);
  assert (xormask () == count);
  CArray pdu;
  pdu.resize (count * 2 + 5);
  pdu[0] = 0x03;
  pdu[1] = 0xD0;
  pdu[2] = count;
  pdu[3] = (addr >> 8) & 0xff;
  pdu[4] = (addr) & 0xff;
  pdu.setpart (andmask.array (), 5, count);
  pdu.setpart (xormask.array (), 5 + count, count);
  return pdu;
}

String
A_MemoryBit_Write_PDU::Decode ()
{
  assert (andmask () == count);
  assert (xormask () == count);
  String s ("A_MemoryBit_Write Len:");
  addHex (s, count);
  s += "Addr: ";
  add16Hex (s, addr);
  s += "And: ";
  for (unsigned i = 0; i < andmask (); i++)
    addHex (s, andmask[i]);
  s += "xor: ";
  for (unsigned i = 0; i < xormask (); i++)
    addHex (s, xormask[i]);
  return s;
}

bool A_MemoryBit_Write_PDU::isResponse(const APDU* req) const
{
  return 0;
}

/* A_UserMemory_Read */

A_UserMemory_Read_PDU::A_UserMemory_Read_PDU ()
{
  addr_extension = 0;
  count = 0;
  addr = 0;
}

A_UserMemory_Read_PDU::A_UserMemory_Read_PDU (const CArray & c)
{
  if (c () != 5)
    throw Exception (PDU_WRONG_FORMAT);
  addr_extension = (c[2] >> 4) & 0xf;
  count = c[2] & 0xf;
  addr = (c[3] << 8) | c[4];
}

CArray
A_UserMemory_Read_PDU::ToPacket ()
{
  assert ((count & 0xf0) == 0);
  assert (addr_extension & 0xf0 == 0);
  CArray pdu;
  pdu.resize (5);
  pdu[0] = 0x02;
  pdu[1] = 0xC0;
  pdu[2] = (addr_extension & 0x0f) << 4 | (count & 0x0f);
  pdu[3] = (addr >> 8) & 0xff;
  pdu[4] = (addr) & 0xff;
  return pdu;
}

String
A_UserMemory_Read_PDU::Decode ()
{
  assert ((count & 0xf0) == 0);
  assert (addr_extension & 0xf0 == 0);
  String s ("A_UserMemory_Read Addr_ext:");
  addHex (s, addr_extension);
  s += " Len: ";
  addHex (s, count);
  s += " Addr: ";
  add16Hex (s, addr);
  return s;
}

bool A_UserMemory_Read_PDU::isResponse(const APDU* req) const
{
  return 0;
}

/* A_UserMemory_Response */

A_UserMemory_Response_PDU::A_UserMemory_Response_PDU ()
{
  addr_extension = 0;
  count = 0;
  addr = 0;
}

A_UserMemory_Response_PDU::A_UserMemory_Response_PDU (const CArray & c)
{
  if (c () < 5)
    throw Exception (PDU_WRONG_FORMAT);
  addr_extension = (c[2] >> 4) & 0xf;
  count = c[2] & 0xf;
  addr = (c[3] << 8) | c[4];
  data.set (c.array () + 5, c () - 5);
  if (data () != count)
    throw Exception (PDU_INCONSISTENT_SIZE);
}

CArray
A_UserMemory_Response_PDU::ToPacket ()
{
  assert ((count & 0xf0) == 0);
  assert ((addr_extension & 0xf0) == 0);
  assert (data () == count);
  CArray pdu;
  pdu.resize (5 + data ());
  pdu[0] = 0x02;
  pdu[1] = 0xC1;
  pdu[2] = (addr_extension & 0x0f) << 4 | (count & 0x0f);
  pdu[3] = (addr >> 8) & 0xff;
  pdu[4] = (addr) & 0xff;
  pdu.setpart (data.array (), 5, data ());
  return pdu;
}

String
A_UserMemory_Response_PDU::Decode ()
{
  assert ((count & 0xf0) == 0);
  assert ((addr_extension & 0xf0) == 0);
  assert (data () == count);
  String s ("A_UserMemory_Response Addr_ext:");
  addHex (s, addr_extension);
  s += " Len: ";
  addHex (s, count);
  s += " Addr: ";
  add16Hex (s, addr);
  s += " Data: ";
  for (unsigned i = 0; i < data (); i++)
    addHex (s, data[i]);
  return s;
}

bool A_UserMemory_Response_PDU::isResponse(const APDU* req) const
{
  if(req->getType()!=A_UserMemory_Read)
    return 0;
  const A_UserMemory_Read_PDU* a=(const A_UserMemory_Read_PDU*)req;
  if(a->count!=count)
    return 0;
  if(a->addr!=addr)
    return 0;
  if(a->addr_extension!=addr_extension)
    return 0;
  return 1;
}

/* A_UserMemory_Write */

A_UserMemory_Write_PDU::A_UserMemory_Write_PDU ()
{
  addr_extension = 0;
  count = 0;
  addr = 0;
}

A_UserMemory_Write_PDU::A_UserMemory_Write_PDU (const CArray & c)
{
  if (c () < 5)
    throw Exception (PDU_WRONG_FORMAT);
  addr_extension = (c[2] >> 4) & 0xf;
  count = c[2] & 0xf;
  addr = (c[3] << 8) | c[4];
  data.set (c.array () + 5, c () - 5);
  if (data () != count)
    throw Exception (PDU_INCONSISTENT_SIZE);
}

CArray
A_UserMemory_Write_PDU::ToPacket ()
{
  assert ((count & 0xf0) == 0);
  assert ((addr_extension & 0xf0) == 0);
  assert (data () == count);
  CArray pdu;
  pdu.resize (5 + data ());
  pdu[0] = 0x02;
  pdu[1] = 0xC2;
  pdu[2] = (addr_extension & 0x0f) << 4 | (count & 0x0f);
  pdu[3] = (addr >> 8) & 0xff;
  pdu[4] = (addr) & 0xff;
  pdu.setpart (data.array (), 5, data ());
  return pdu;
}

String
A_UserMemory_Write_PDU::Decode ()
{
  assert ((count & 0xf0) == 0);
  assert ((addr_extension & 0xf0) == 0);
  assert (data () == count);
  String s ("A_UserMemory_Write Addr_ext:");
  addHex (s, addr_extension);
  s += " Len: ";
  addHex (s, count);
  s += " Addr: ";
  add16Hex (s, addr);
  s += " Data: ";
  for (unsigned i = 0; i < data (); i++)
    addHex (s, data[i]);
  return s;
}

bool A_UserMemory_Write_PDU::isResponse(const APDU* req) const
{
  return 0;
}

/* A_UserMemoryBit_Write */

A_UserMemoryBit_Write_PDU::A_UserMemoryBit_Write_PDU ()
{
  addr_extension = 0;
  count = 0;
  addr = 0;
}

A_UserMemoryBit_Write_PDU::A_UserMemoryBit_Write_PDU (const CArray & c)
{
  if (c () < 5)
    throw Exception (PDU_WRONG_FORMAT);
  count = c[2];
  addr = (c[3] << 8) | c[4];
  if (c () - 5 != count * 2)
    throw Exception (PDU_INCONSISTENT_SIZE);
  andmask.set (c.array () + 5, count);
  xormask.set (c.array () + 5 + count, count);
}

CArray
A_UserMemoryBit_Write_PDU::ToPacket ()
{
  assert (andmask () == count);
  assert (xormask () == count);
  CArray pdu;
  pdu.resize (5 + 2 * count);
  pdu[0] = 0x02;
  pdu[1] = 0xC4;
  pdu[2] = count;
  pdu[3] = (addr >> 8) & 0xff;
  pdu[4] = (addr) & 0xff;
  pdu.setpart (andmask.array (), 5, count);
  pdu.setpart (xormask.array (), 5 + count, count);
  return pdu;
}

String
A_UserMemoryBit_Write_PDU::Decode ()
{
  assert (andmask () == count);
  assert (xormask () == count);
  String s ("A_UserMemoryBit_Write Len:");
  addHex (s, count);
  s += "Addr: ";
  add16Hex (s, addr);
  s += "And: ";
  for (unsigned i = 0; i < andmask (); i++)
    addHex (s, andmask[i]);
  s += "xor: ";
  for (unsigned i = 0; i < xormask (); i++)
    addHex (s, xormask[i]);
  return s;
}

bool A_UserMemoryBit_Write_PDU::isResponse(const APDU* req) const
{
  return 0;
}

/* A_UserManufacturerInfo_Read */

A_UserManufacturerInfo_Read_PDU::A_UserManufacturerInfo_Read_PDU ()
{
}

A_UserManufacturerInfo_Read_PDU::
A_UserManufacturerInfo_Read_PDU (const CArray & c)
{
  if (c () != 2)
    throw Exception (PDU_WRONG_FORMAT);
}

CArray
A_UserManufacturerInfo_Read_PDU::ToPacket ()
{
  CArray pdu;
  pdu.resize (2);
  pdu[0] = 0x02;
  pdu[1] = 0xC5;
  return pdu;
}

String
A_UserManufacturerInfo_Read_PDU::Decode ()
{
  String s ("A_UserManufacturerInfo_Read");
  return s;
}

bool A_UserManufacturerInfo_Read_PDU::isResponse(const APDU* req) const
{
  return 0;
}

/* A_UserManufacturerInfo_Response */

A_UserManufacturerInfo_Response_PDU::A_UserManufacturerInfo_Response_PDU ()
{
  manufacturerid = 0;
  data = 0;
}

A_UserManufacturerInfo_Response_PDU::
A_UserManufacturerInfo_Response_PDU (const CArray & c)
{
  if (c () != 5)
    throw Exception (PDU_WRONG_FORMAT);
  manufacturerid = c[2];
  data = (c[3] << 8) | c[4];
}

CArray
A_UserManufacturerInfo_Response_PDU::ToPacket ()
{
  CArray pdu;
  pdu.resize (5);
  pdu[0] = 0x02;
  pdu[1] = 0xC6;
  pdu[2] = manufacturerid;
  pdu[3] = (data >> 8) & 0xff;
  pdu[4] = data & 0xff;
  return pdu;
}

String
A_UserManufacturerInfo_Response_PDU::Decode ()
{
  String s ("A_UserManufactueerInfo_Response Manufacturer:");
  addHex (s, manufacturerid);
  s += " data: ";
  add16Hex (s, data);
  return s;
}

bool A_UserManufacturerInfo_Response_PDU::isResponse(const APDU* req) const
{
  return req->getType()==A_UserManufacturerInfo_Read;
}

/* A_Restart */

A_Restart_PDU::A_Restart_PDU ()
{
}

A_Restart_PDU::A_Restart_PDU (const CArray & c)
{
  if (c () != 2)
    throw Exception (PDU_WRONG_FORMAT);
}

CArray
A_Restart_PDU::ToPacket ()
{
  CArray pdu;
  pdu.resize (2);
  pdu[0] = 0x03;
  pdu[1] = 0x80;
  return pdu;
}

String
A_Restart_PDU::Decode ()
{
  String s ("A_Restart");
  return s;
}

bool A_Restart_PDU::isResponse(const APDU* req) const
{
  return 0;
}

/* A_Authorize_Request */

A_Authorize_Request_PDU::A_Authorize_Request_PDU ()
{
  key = 0;
}

A_Authorize_Request_PDU::A_Authorize_Request_PDU (const CArray & c)
{
  if (c () != 7)
    throw Exception (PDU_WRONG_FORMAT);
  key = (c[3]<<24) | (c[4]<<16) | (c[5]<<8) | (c[6]);
}

CArray
A_Authorize_Request_PDU::ToPacket ()
{
  CArray pdu;
  pdu.resize (7);
  pdu[0] = 0x03;
  pdu[1] = 0xD1;
  pdu[2] = 0x00;
  pdu[3] = (key >> 24) & 0xff;
  pdu[4] = (key >> 16) & 0xff;
  pdu[5] = (key >> 8) & 0xff;
  pdu[6] = (key) & 0xff;
  return pdu;
}

String
A_Authorize_Request_PDU::Decode ()
{
  String s ("A_Authorize_Request Key:");
  return s + FormatEIBKey (key);
}

bool A_Authorize_Request_PDU::isResponse(const APDU* req) const
{
  return 0;
}

/* A_Authorize_Response */

A_Authorize_Response_PDU::A_Authorize_Response_PDU ()
{
  level = 0;
}

A_Authorize_Response_PDU::A_Authorize_Response_PDU (const CArray & c)
{
  if (c () != 3)
    throw Exception (PDU_WRONG_FORMAT);
  level = c[2];
}

CArray
A_Authorize_Response_PDU::ToPacket ()
{
  CArray pdu;
  pdu.resize (3);
  pdu[0] = 0x03;
  pdu[1] = 0xD2;
  pdu[2] = level;
  return pdu;
}

String
A_Authorize_Response_PDU::Decode ()
{
  String s ("A_Authorize_Response Level:");
  addHex (s, level);
  return s;
}

bool A_Authorize_Response_PDU::isResponse(const APDU* req) const
{
  return req->getType()==A_Authorize_Request;
}

/* A_Key_Write */

A_Key_Write_PDU::A_Key_Write_PDU ()
{
  key = 0;
  level = 0;
}

A_Key_Write_PDU::A_Key_Write_PDU (const CArray & c)
{
  if (c () != 7)
    throw Exception (PDU_WRONG_FORMAT);
  level = c[2];
  key = (c[3]<<24) | (c[4]<<16) | (c[5]<<8) | (c[6]);
}

CArray
A_Key_Write_PDU::ToPacket ()
{
  CArray pdu;
  pdu.resize (7);
  pdu[0] = 0x03;
  pdu[1] = 0xD3;
  pdu[2] = level;
  pdu[3] = (key >> 24) & 0xff;
  pdu[4] = (key >> 16) & 0xff;
  pdu[5] = (key >> 8) & 0xff;
  pdu[6] = (key) & 0xff;
  return pdu;
}

String
A_Key_Write_PDU::Decode ()
{
  String s ("A_Key_Write Level:");
  addHex (s, level);
  s += " Key: ";
  return s + FormatEIBKey (key);
}

bool A_Key_Write_PDU::isResponse(const APDU* req) const
{
  return 0;
}

/* A_Key_Response */

A_Key_Response_PDU::A_Key_Response_PDU ()
{
  level = 0;
}

A_Key_Response_PDU::A_Key_Response_PDU (const CArray & c)
{
  if (c () != 3)
    throw Exception (PDU_WRONG_FORMAT);
  level = c[2];
}

CArray
A_Key_Response_PDU::ToPacket ()
{
  CArray pdu;
  pdu.resize (3);
  pdu[0] = 0x03;
  pdu[1] = 0xD4;
  pdu[2] = level;
  return pdu;
}

String
A_Key_Response_PDU::Decode ()
{
  String s ("A_Key_Response Level:");
  addHex (s, level);
  return s;
}

bool A_Key_Response_PDU::isResponse(const APDU* req) const
{
  if(req->getType()!=A_Key_Write)
    return 0;
  const A_Key_Write_PDU* a=(const A_Key_Write_PDU*)req;
  return a->level==level;
}
