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

#include "emi1.h"

bool
EMI1Layer2Interface::addAddress (eibaddr_t addr)
{
  return 0;
}

bool
EMI1Layer2Interface::addGroupAddress (eibaddr_t addr)
{
  return 0;
}

bool
EMI1Layer2Interface::removeAddress (eibaddr_t addr)
{
  return 0;
}

bool
EMI1Layer2Interface::removeGroupAddress (eibaddr_t addr)
{
  return 0;
}

bool
EMI1Layer2Interface::Connection_Lost ()
{
  return iface->Connection_Lost ();
}

eibaddr_t
EMI1Layer2Interface::getDefaultAddr ()
{
  return 0;
}

bool
EMI1Layer2Interface::openVBusmonitor ()
{
  vmode = 1;
  return 1;
}

bool
EMI1Layer2Interface::closeVBusmonitor ()
{
  vmode = 0;
  return 1;
}

EMI1Layer2Interface::EMI1Layer2Interface (LowLevelDriverInterface * i,
					  Trace * tr)
{
  tr->TracePrintf (2, this, "Open");
  iface = i;
  t = tr;
  mode = 0;
  vmode = 0;
  pth_sem_init (&out_signal);
  getwait = pth_event (PTH_EVENT_SEM, &out_signal);
  Start ();
  tr->TracePrintf (2, this, "Opened");
}

EMI1Layer2Interface::~EMI1Layer2Interface ()
{
  t->TracePrintf (2, this, "Destroy");
  Stop ();
  pth_event_free (getwait, PTH_FREE_THIS);
  while (!outqueue.isempty ())
    delete outqueue.get ();
  delete iface;
}

bool
EMI1Layer2Interface::enterBusmonitor ()
{
  const uchar t1[] = { 0x46, 0x01, 0x00, 0x60, 0x90 };
  t->TracePrintf (2, this, "OpenBusmon");
  if (mode != 0)
    return 0;
  iface->SendReset ();
  iface->Send_Packet (CArray (t1, sizeof (t1)));

  if (!iface->Send_Queue_Empty ())
    {
      pth_event_t
	e = pth_event (PTH_EVENT_SEM, iface->Send_Queue_Empty_Cond ());
      pth_wait (e);
      pth_event_free (e, PTH_FREE_THIS);
    }
  mode = 1;
  return 1;
}

bool
EMI1Layer2Interface::leaveBusmonitor ()
{
  if (mode != 1)
    return 0;
  t->TracePrintf (2, this, "CloseBusmon");
  uchar t[] =
  {
  0x46, 0x01, 0x00, 0x60, 0xc0};
  iface->Send_Packet (CArray (t, sizeof (t)));
  while (!iface->Send_Queue_Empty ())
    {
      pth_event_t
	e = pth_event (PTH_EVENT_SEM, iface->Send_Queue_Empty_Cond ());
      pth_wait (e);
      pth_event_free (e, PTH_FREE_THIS);
    }
  mode = 0;
  return 1;
}

bool
EMI1Layer2Interface::Open ()
{
  const uchar t1[] = { 0x46, 0x01, 0x00, 0x60, 0x12 };
  t->TracePrintf (2, this, "OpenL2");
  if (mode != 0)
    return 0;
  iface->SendReset ();
  iface->Send_Packet (CArray (t1, sizeof (t1)));

  while (!iface->Send_Queue_Empty ())
    {
      pth_event_t
	e = pth_event (PTH_EVENT_SEM, iface->Send_Queue_Empty_Cond ());
      pth_wait (e);
      pth_event_free (e, PTH_FREE_THIS);
    }
  mode = 2;
  return 1;
}

bool
EMI1Layer2Interface::Close ()
{
  if (mode != 2)
    return 0;
  t->TracePrintf (2, this, "CloseL2");
  uchar t[] =
  {
  0x46, 0x01, 0x00, 0x60, 0xc0};
  iface->Send_Packet (CArray (t, sizeof (t)));
  if (!iface->Send_Queue_Empty ())
    {
      pth_event_t
	e = pth_event (PTH_EVENT_SEM, iface->Send_Queue_Empty_Cond ());
      pth_wait (e);
      pth_event_free (e, PTH_FREE_THIS);
    }
  mode = 0;
  return 1;
}

bool
EMI1Layer2Interface::Send_Queue_Empty ()
{
  return iface->Send_Queue_Empty ();
}


void
EMI1Layer2Interface::Send_L_Data (LPDU * l)
{
  t->TracePrintf (2, this, "Send %s", l->Decode ()());
  if (l->getType () != L_Data)
    {
      delete l;
      return;
    }
  L_Data_PDU *l1 = (L_Data_PDU *) l;
  assert (l1->data () >= 1);
  assert (l1->data () <= 0xf);
  assert ((l1->hopcount & 0xf8) == 0);

  CArray pdu;
  uchar c;
  switch (l1->prio)
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
  pdu.resize (l1->data () + 7);
  pdu[0] = 0x11;
  pdu[1] = c << 2;
  pdu[2] = 0;
  pdu[3] = 0;
  pdu[4] = (l1->dest >> 8) & 0xff;
  pdu[5] = (l1->dest) & 0xff;
  pdu[6] =
    (l1->hopcount & 0x07) << 4 | ((l1->data () - 1) & 0x0f) | (l1->AddrType ==
							       GroupAddress ?
							       0x80 : 0x00);
  pdu.setpart (l1->data.array (), 7, l1->data ());
  iface->Send_Packet (pdu);
  if (vmode)
    {
      L_Busmonitor_PDU *l2 = new L_Busmonitor_PDU;
      l2->pdu.set (l->ToPacket ());
      outqueue.put (l2);
      pth_sem_inc (&out_signal, 1);
    }
  delete l;
}

LPDU *
EMI1Layer2Interface::Get_L_Data (pth_event_t stop)
{
  if (stop != NULL)
    pth_event_concat (getwait, stop, NULL);

  pth_wait (getwait);

  if (stop)
    pth_event_isolate (getwait);

  if (pth_event_status (getwait) == PTH_STATUS_OCCURRED)
    {
      pth_sem_dec (&out_signal);
      LPDU *l = outqueue.get ();
      t->TracePrintf (2, this, "Recv %s", l->Decode ()());
      return l;
    }
  else
    return 0;
}

void
EMI1Layer2Interface::Run (pth_sem_t * stop1)
{
  pth_event_t stop = pth_event (PTH_EVENT_SEM, stop1);
  while (pth_event_status (stop) != PTH_STATUS_OCCURRED)
    {
      CArray *c = iface->Get_Packet (stop);
      if (!c)
	continue;
      if (c->len () > 7 && (*c)[0] == 0x49 && mode == 2)
	{
	  unsigned len;
	  L_Data_PDU *p = new L_Data_PDU;
	  p->source = ((*c)[2] << 8) | ((*c)[3]);
	  p->dest = ((*c)[4] << 8) | ((*c)[5]);
	  switch (((*c)[1] >> 2) & 0x3)
	    {
	    case 0:
	      p->prio = PRIO_SYSTEM;
	      break;
	    case 1:
	      p->prio = PRIO_URGENT;
	      break;
	    case 2:
	      p->prio = PRIO_NORMAL;
	      break;
	    case 3:
	      p->prio = PRIO_LOW;
	      break;
	    }
	  p->AddrType = ((*c)[6] & 0x80) ? GroupAddress : IndividualAddress;
	  if (p->AddrType == IndividualAddress)
	    p->dest = 0;
	  len = ((*c)[6] & 0x0f) + 1;
	  if (len > c->len () - 7)
	    len = c->len () - 7;
	  p->data.set (c->array () + 7, len);
	  p->hopcount = ((*c)[6] >> 4) & 0x07;
	  delete c;
	  t->TracePrintf (2, this, "Recv %s", p->Decode ()());
	  if (vmode)
	    {
	      L_Busmonitor_PDU *l2 = new L_Busmonitor_PDU;
	      l2->pdu.set (p->ToPacket ());
	      outqueue.put (l2);
	      pth_sem_inc (&out_signal, 1);
	    }
	  outqueue.put (p);
	  pth_sem_inc (&out_signal, 1);
	  continue;
	}
      if (c->len () > 4 && (*c)[0] == 0x49 && mode == 1)
	{
	  L_Busmonitor_PDU *p = new L_Busmonitor_PDU;
	  p->pdu.set (c->array () + 4, c->len () - 4);
	  delete c;
	  t->TracePrintf (2, this, "Recv %s", p->Decode ()());
	  outqueue.put (p);
	  pth_sem_inc (&out_signal, 1);
	  continue;
	}
      delete c;
    }
  pth_event_free (stop, PTH_FREE_THIS);
}