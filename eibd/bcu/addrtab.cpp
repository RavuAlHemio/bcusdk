/*
    EIBD eib bus access and management daemon
    Copyright (C) 2005 Martin Kögler <mkoegler@auto.tuwien.ac.at>

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

#include "addrtab.h"

const uchar EMI2_TLL[] = { 0xA9, 0x00, 0x12, 0x34, 0x56, 0x78, 0x0A };
const uchar EMI2_NORM[] = { 0xA9, 0x00, 0x12, 0x34, 0x56, 0x78, 0x8A };
const uchar EMI2_LCON[] = { 0x43, 0x00, 0x00, 0x00, 0x00, 0x00 };
const uchar EMI2_LDIS[] = { 0x44, 0x00, 0x00, 0x00, 0x00, 0x00 };
const uchar EMI2_READ[] =
  { 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x02, 0x01, 0x01, 0x16 };

const uchar EMI1_READ[] = { 0x4C, 0x01, 0x01, 0x16 };


static void
llwait (LowLevelDriverInterface * iface)
{
  if (!iface->Send_Queue_Empty ())
    {
      pth_event_t
	e = pth_event (PTH_EVENT_SEM, iface->Send_Queue_Empty_Cond ());
      pth_wait (e);
      pth_event_free (e, PTH_FREE_THIS);
    }
}

int
readEMI1AddrTabSize (LowLevelDriverInterface * iface, uchar & result)
{
  CArray *d1, d;
  iface->SendReset ();
  iface->Send_Packet (CArray (EMI1_READ, sizeof (EMI1_READ)));
  llwait (iface);
  d1 = iface->Get_Packet (0);
  if (!d1)
    return 0;
  d = *d1;
  delete d1;
  if (d () != 5)
    return 0;
  if (d[0] != 0x4B)
    return 0;
  if (d[1] != 0x01)
    return 0;
  if (d[2] != 0x01)
    return 0;
  if (d[3] != 0x16)
    return 0;
  result = d[4];
  return 1;
}

int
writeEMI1AddrTabSize (LowLevelDriverInterface * iface, uchar size)
{
  uchar res;
  CArray d;
  iface->SendReset ();
  d.resize (5);
  d[0] = 0x46;
  d[1] = 0x01;
  d[2] = 0x01;
  d[3] = 0x16;
  d[4] = size;
  iface->Send_Packet (d);
  llwait (iface);
  if (!readEMI1AddrTabSize (iface, res))
    return 0;
  return res == size;
}

int
readEMI2AddrTabSize (LowLevelDriverInterface * iface, uchar & result)
{
  CArray *d1, d;
  iface->SendReset ();
  iface->Send_Packet (CArray (EMI2_TLL, sizeof (EMI2_TLL)));
  iface->Send_Packet (CArray (EMI2_LCON, sizeof (EMI2_LCON)));
  iface->Send_Packet (CArray (EMI2_READ, sizeof (EMI2_READ)));
  // ignore ACKs
  d1 = iface->Get_Packet (0);
  if (d1)
    delete d1;
  else
    return 0;

  d1 = iface->Get_Packet (0);
  if (d1)
    delete d1;
  else
    return 0;

  d1 = iface->Get_Packet (0);
  if (!d1)
    return 0;
  d = *d1;
  delete d1;
  if (d () != 12)
    return 0;
  if (d[0] != 0x89)
    return 0;
  if (d[1] != 0x00)
    return 0;
  if (d[2] != 0x00)
    return 0;
  if (d[3] != 0x00)
    return 0;
  if (d[4] != 0x00)
    return 0;
  if (d[5] != 0x00)
    return 0;
  if (d[6] != 0x04)
    return 0;
  if (d[7] != 0x42)
    return 0;
  if (d[8] != 0x41)
    return 0;
  if (d[9] != 0x01)
    return 0;
  if (d[10] != 0x16)
    return 0;
  result = d[11];
  iface->Send_Packet (CArray (EMI2_LDIS, sizeof (EMI2_LDIS)));
  d1 = iface->Get_Packet (0);
  if (!d1)
    return 0;
  else
    delete d1;
  iface->Send_Packet (CArray (EMI2_NORM, sizeof (EMI2_NORM)));
  llwait (iface);
  return 1;
}

int
writeEMI2AddrTabSize (LowLevelDriverInterface * iface, uchar size)
{
  uchar res;
  CArray *d1, d;
  iface->SendReset ();
  iface->Send_Packet (CArray (EMI2_TLL, sizeof (EMI2_TLL)));

  iface->Send_Packet (CArray (EMI2_LCON, sizeof (EMI2_LCON)));
  // ignore ACKs
  d1 = iface->Get_Packet (0);
  if (d1)
    delete d1;
  else
    return 0;

  d.resize (12);
  d[0] = 0x41;
  d[1] = 0x00;
  d[2] = 0x00;
  d[3] = 0x00;
  d[4] = 0x00;
  d[5] = 0x00;
  d[6] = 0x03;
  d[7] = 0x02;
  d[8] = 0x81;
  d[9] = 0x01;
  d[10] = 0x16;
  d[11] = size;

  iface->Send_Packet (d);
  d1 = iface->Get_Packet (0);
  if (d1)
    delete d1;
  else
    return 0;

  iface->Send_Packet (CArray (EMI2_LDIS, sizeof (EMI2_LDIS)));
  d1 = iface->Get_Packet (0);
  if (!d1)
    return 0;
  else
    delete d1;
  iface->Send_Packet (CArray (EMI2_NORM, sizeof (EMI2_NORM)));
  llwait (iface);

  if (!readEMI2AddrTabSize (iface, res))
    return 0;
  return res == size;

  return 1;
}

int
readAddrTabSize (LowLevelDriverInterface * iface, uchar & result)
{
  switch (iface->getEMIVer ())
    {
    case LowLevelDriverInterface::vEMI1:
      return readEMI1AddrTabSize (iface, result);
    case LowLevelDriverInterface::vEMI2:
      return readEMI2AddrTabSize (iface, result);

    default:
      return 0;
    }
}

int
writeAddrTabSize (LowLevelDriverInterface * iface, uchar size)
{
  switch (iface->getEMIVer ())
    {
    case LowLevelDriverInterface::vEMI1:
      return writeEMI1AddrTabSize (iface, size);
    case LowLevelDriverInterface::vEMI2:
      return writeEMI2AddrTabSize (iface, size);

    default:
      return 0;
    }
}
